#include "DS2431.h"

DS2431::DS2431(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(scratchpad) < 256, "Implementation does not cover the whole address-space");
    static_assert(sizeof(memory) < 256,  "Implementation does not cover the whole address-space");

    clearMemory();
    clearScratchpad();

    page_protection = 0;
    page_eprom_mode = 0;

    updatePageStatus();
}

void DS2431::duty(OneWireHub * const hub)
{
    constexpr uint8_t ALTERNATING_10 { 0xAA };
    static uint16_t   reg_TA         { 0 }; // contains TA1, TA2
    static uint8_t    reg_ES         { 0 }; // E/S register
    uint16_t          crc            { 0 };

    uint8_t  page_offset { 0 };
    uint8_t  cmd, data;
    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0x0F:      // WRITE SCRATCHPAD COMMAND

            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc))  return;
            reg_ES = uint8_t(reg_TA) & SCRATCHPAD_MASK;
            page_offset = reg_ES;

            // receive up to 8 bytes of data
            for (; reg_ES < SCRATCHPAD_SIZE; ++reg_ES)
            {
                if (hub->recv(&scratchpad[reg_ES], 1, crc))
                {
                    if (hub->getError() == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH) reg_ES |= REG_ES_PF_MASK;
                    break;
                }
            }
            reg_ES--;
            reg_ES &= SCRATCHPAD_MASK;

            if (hub->getError() == Error::NO_ERROR)  // try to send crc if wanted
            {
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc), 2);
            }

            if (reg_TA < (4*PAGE_SIZE)) // check if page is protected or in eprom-mode
            {
                const uint8_t position = uint8_t(reg_TA) & ~SCRATCHPAD_MASK;
                if (getPageProtection(reinterpret_cast<uint8_t *>(&reg_TA)[0]))       // protected: load memory-segment to scratchpad
                {
                    for (uint8_t i = 0; i < SCRATCHPAD_SIZE; ++i) scratchpad[i] = memory[position + i];
                }
                else if (getPageEpromMode(reinterpret_cast<uint8_t *>(&reg_TA)[0]))   // eprom: logical AND of memory and data
                {
                    for (uint8_t i = page_offset; i <= reg_ES; ++i) scratchpad[i] &= memory[position + i];
                }
            }
            break;

        case 0xAA:      // READ SCRATCHPAD COMMAND

            if (hub->send(reinterpret_cast<uint8_t *>(&reg_TA),2,crc))  return;
            if (hub->send(&reg_ES,1,crc)) return;

            {   // send Scratchpad content
                const uint8_t start  = uint8_t(reg_TA) & SCRATCHPAD_MASK;
                const uint8_t length = (reg_ES & SCRATCHPAD_MASK)+ uint8_t(1) - start; // difference to ds2433
                if (hub->send(&scratchpad[start],length,crc))   return;
            }

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break; // send 1s when read is complete, is passive, so do nothing

        case 0x55:      // COPY SCRATCHPAD COMMAND

            if (hub->recv(&data))                                  return;
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[0])   break;
            if (hub->recv(&data))                                  return;
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[1])   break;
            if (hub->recv(&data))                                  return;
            if (data != reg_ES)                                    return; // Auth code must match

            if (getPageProtection(uint8_t(reg_TA)))                break; // stop if page is protected (WriteMemory also checks this)
            if ((reg_ES & REG_ES_PF_MASK) != 0)                    break; // stop if error occured earlier

            reg_ES |= REG_ES_AA_MASK; // compare was successful

            reg_TA &= ~uint16_t(SCRATCHPAD_MASK);

            // Write Scratchpad to memory, writing takes about 10ms
            writeMemory(scratchpad, SCRATCHPAD_SIZE, reinterpret_cast<uint8_t *>(&reg_TA)[0]); // checks if copy protected

            noInterrupts();

            do
            {
                hub->clearError();

                hub->sendBit(true); // send passive 1s

            }
            while   (hub->getError() == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH); // wait for timeslots

            interrupts();

            while (!hub->send(&ALTERNATING_10)); //  alternating 1 & 0 after copy is complete
            break;

        case 0xF0:      // READ MEMORY COMMAND

            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2))  return;
            if (reg_TA >= MEM_SIZE) return;
            if (hub->send(&memory[reg_TA],MEM_SIZE - uint8_t(reg_TA),crc)) return;
            break; // send 1s when read is complete, is passive, so do nothing here

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2431::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0x00), sizeof(memory));
}

void DS2431::clearScratchpad(void)
{
    memset(scratchpad, static_cast<uint8_t>(0x00), SCRATCHPAD_SIZE);
}

bool DS2431::writeMemory(const uint8_t* const source, const uint8_t length, const uint8_t position)
{
    for (uint8_t i = 0; i < length; ++i) {
        if ((position + i) >= sizeof(memory)) break;
        if (getPageProtection(position + i)) continue;
        memory[position + i] = source[i];
    }

    if ((position+length) > 127) updatePageStatus();

    return true;
}

bool DS2431::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}

void DS2431::setPageProtection(const uint8_t position)
{
    if      (position < 1*PAGE_SIZE)    memory[0x80] = WP_MODE;
    else if (position < 2*PAGE_SIZE)    memory[0x81] = WP_MODE;
    else if (position < 3*PAGE_SIZE)    memory[0x82] = WP_MODE;
    else if (position < 4*PAGE_SIZE)    memory[0x83] = WP_MODE;
    else if (position < 0x85)           memory[0x84] = WP_MODE;
    else if (position == 0x85)          memory[0x85] = WP_MODE;
    else if (position < 0x88)           memory[0x85] = EP_MODE;

    updatePageStatus();
}

bool DS2431::getPageProtection(const uint8_t position) const
{
    // should be an accurate model of the control bytes
    if      (position < 1*PAGE_SIZE)
    {
        if ((page_protection & 1) != 0) return true;
    }
    else if (position < 2*PAGE_SIZE)
    {
        if ((page_protection & 2) != 0) return true;
    }
    else if (position < 3*PAGE_SIZE)
    {
        if ((page_protection & 4) != 0) return true;
    }
    else if (position < 4*PAGE_SIZE)
    {
        if ((page_protection & 8) != 0) return true;
    }
    else if (position == 0x80)
    {
        if (((page_protection & (1 + 16)) != 0) || ((page_eprom_mode & 1)) != 0) return true;
    }
    else if (position == 0x81)
    {
        if (((page_protection & (2 + 16)) != 0) || ((page_eprom_mode & 2)) != 0) return true;
    }
    else if (position == 0x82)
    {
        if (((page_protection & (4 + 16)) != 0) || ((page_eprom_mode & 4)) != 0) return true;
    }
    else if (position == 0x83)
    {
        if (((page_protection & (8 + 16)) != 0) || ((page_eprom_mode & 8)) != 0) return true;
    }
    else if (position == 0x85)
    {
        if ((page_protection & (32+16)) != 0) return true;
    }
    else if ((position == 0x86) || (position == 0x87))
    {
        if ((page_protection & (64+16)) != 0) return true;
    }
    else if (position > 127) // filter the rest
    {
        if ((page_protection & 16) != 0) return true;
    }
    return false;
}

void DS2431::setPageEpromMode(const uint8_t position)
{
    if      (position < 1*PAGE_SIZE)  memory[0x80] = EP_MODE;
    else if (position < 2*PAGE_SIZE)  memory[0x81] = EP_MODE;
    else if (position < 3*PAGE_SIZE)  memory[0x82] = EP_MODE;
    else if (position < 4*PAGE_SIZE)  memory[0x83] = EP_MODE;
    updatePageStatus();
}

bool DS2431::getPageEpromMode(const uint8_t position) const
{
    if      (position < 1*PAGE_SIZE)
    {
        if ((page_eprom_mode & 1) != 0) return true;
    }
    else if (position < 2*PAGE_SIZE)
    {
        if ((page_eprom_mode & 2) != 0) return true;
    }
    else if (position < 3*PAGE_SIZE)
    {
        if ((page_eprom_mode & 4) != 0) return true;
    }
    else if (position < 4*PAGE_SIZE)
    {
        if ((page_eprom_mode & 8) != 0) return true;
    }
    return false;
}


bool DS2431::updatePageStatus(void)
{
    page_eprom_mode = 0;
    page_protection = 0;

    if (memory[0x80] == WP_MODE) page_protection |= 1;
    if (memory[0x81] == WP_MODE) page_protection |= 2;
    if (memory[0x82] == WP_MODE) page_protection |= 4;
    if (memory[0x83] == WP_MODE) page_protection |= 8;

    if (memory[0x84] == WP_MODE) page_protection |= 16;
    if (memory[0x84] == EP_MODE) page_protection |= 16;

    if (memory[0x85] == WP_MODE) page_protection |= 32; // only byte x85
    if (memory[0x85] == EP_MODE) page_protection |= 64+32; // also byte x86 x87

    if (memory[0x80] == EP_MODE) page_eprom_mode |= 1;
    if (memory[0x81] == EP_MODE) page_eprom_mode |= 2;
    if (memory[0x82] == EP_MODE) page_eprom_mode |= 4;
    if (memory[0x83] == EP_MODE) page_eprom_mode |= 8;
    return true;
}
