#include "DS2433.h"

DS2433::DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 65535,  "Implementation does not cover the whole address-space");
    clearMemory();
    clearScratchpad();
}

void DS2433::duty(OneWireHub * const hub)
{
    constexpr uint8_t ALTERNATE_01 { 0b10101010 };

    static uint16_t reg_TA { 0 }; // contains TA1, TA2 (Target Address)
    static uint8_t  reg_ES { 31 };  // E/S register

    uint8_t  data, cmd;
    uint16_t crc { 0 };

    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0x0F:      // WRITE SCRATCHPAD COMMAND

            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) return;
            reg_TA &= MEM_MASK; // make sure to stay in boundary
            reg_ES = uint8_t(reg_TA) & PAGE_MASK; // register-offset

            // receive up to 8 bytes of data
            for (; reg_ES < PAGE_SIZE; ++reg_ES)
            {
                if (hub->recv(&scratchpad[reg_ES], 1, crc))
                {
                    if (hub->getError() == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH) reg_ES |= REG_ES_PF_MASK;
                    break;
                }
            }
            reg_ES--;
            reg_ES &= PAGE_MASK;

            if (hub->getError() == Error::NO_ERROR)  // try to send crc if wanted
            {
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc), 2);
            }
            break;

        case 0x55:      // COPY SCRATCHPAD

            if (hub->recv(&data)) return; // TA1
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[0]) return;
            if (hub->recv(&data)) return;  // TA2
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[1]) return;
            if (hub->recv(&data)) return;  // ES
            if (data != reg_ES) return;

            if ((reg_ES & REG_ES_PF_MASK) != 0)                  break; // stop if error occured earlier

            reg_ES |= REG_ES_AA_MASK; // compare was successful

            {    // Write Scratchpad to memory, writing takes about 5ms
                const uint8_t start  = uint8_t(reg_TA) & PAGE_MASK;
                const uint8_t length = (reg_ES & PAGE_MASK)+ uint8_t(1) - start;
                writeMemory(&scratchpad[start], length, reg_TA);
            }

            noInterrupts();

            do
            {
                hub->clearError();
                hub->sendBit(true); // send passive 1s
            }
            while   (hub->getError() == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH); // wait for timeslots

            interrupts();

            while (!hub->send(&ALTERNATE_01)); // send alternating 1 & 0 after copy is complete
            break;

        case 0xAA:      // READ SCRATCHPAD COMMAND

            if (hub->send(reinterpret_cast<uint8_t *>(&reg_TA),2))  return;
            if (hub->send(&reg_ES,1)) return;

            {   // send Scratchpad content
                const uint8_t start  = uint8_t(reg_TA) & PAGE_MASK;
                const uint8_t length = PAGE_SIZE - start;
                if (hub->send(&scratchpad[start],length))   return;
            }
            return; // datasheed says we should send all 1s, till reset (1s are passive... so nothing to do here)

        case 0xF0:      // READ MEMORY

            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2)) return;

            for (uint16_t i = reg_TA; i < MEM_SIZE; i+=PAGE_SIZE) // model of the 32byte scratchpad
            {
                if (hub->send(&memory[i],PAGE_SIZE)) return;
            }
            return; // datasheed says we should send all 1s, till reset (1s are passive... so nothing to do here)

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2433::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0x00), MEM_SIZE);
}

void DS2433::clearScratchpad(void)
{
    memset(scratchpad, static_cast<uint8_t>(0x00), PAGE_SIZE);
}

bool DS2433::writeMemory(const uint8_t* const source, const uint16_t length, const uint16_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);
    return true;
}

bool DS2433::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}
