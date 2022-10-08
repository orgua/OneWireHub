#include "DS2430.h"

DS2430::DS2430(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(scratchpad) < 256, "Implementation does not cover the whole address-space");
    static_assert(sizeof(memory) < 256,  "Implementation does not cover the whole address-space");

    clearMemory();
    clearScratchpad();
}

void DS2430::duty(OneWireHub * const hub)
{
    static uint8_t    reg_TA         { 0 }; // contains TA1, TA2
    static uint8_t    reg_ES         { 0 }; // E/S register

    uint8_t  cmd, data;
    if (hub->recv(&cmd,1)) return;
    switch (cmd)
    {
        case 0x0F:      // WRITE SCRATCHPAD COMMAND

            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),1))  return;
            reg_ES = uint8_t(reg_TA) & SCRATCHPAD_MASK;
            scratchpad_start_address = reg_ES;

            // receive up to 32 bytes of data
            for (; reg_ES < SCRATCHPAD_SIZE; ++reg_ES)
            {
                if (hub->recv(&scratchpad[reg_ES], 1))
                {
                    if (hub->getError() == Error::AWAIT_TIMESLOT_TIMEOUT_HIGH) reg_ES |= REG_ES_PF_MASK;
                    break;
                }
            }
            reg_ES--;
            scratchpad_size = scratchpad_start_address;
            reg_ES &= SCRATCHPAD_MASK;

            break;

        case 0xAA:      // READ SCRATCHPAD COMMAND

            if (hub->send(reinterpret_cast<uint8_t *>(&reg_TA), 1)) return;

            {   // send Scratchpad content
                const uint8_t start  = uint8_t(reg_TA) & SCRATCHPAD_MASK;
                const uint8_t length = SCRATCHPAD_MASK - start;
                if (hub->send(&scratchpad[start],length))   return;
            }

            break; // send 1s when read is complete, is passive, so do nothing

        case 0x55:      // COPY SCRATCHPAD COMMAND

            if (hub->recv(&data))                                  return;
            if (data != 0xA5)                                      break;

            writeMemory(scratchpad, scratchpad_size, scratchpad_start_address);

            break;

        case 0xF0:      // READ MEMORY COMMAND
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),1))  return;

            if (reg_TA >= MEM_SIZE) return;
            if (hub->send(&memory[reg_TA],MEM_SIZE - uint8_t(reg_TA))) return;
            break; // send 1s when read is complete, is passive, so do nothing here

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2430::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0x00), sizeof(memory));
}

void DS2430::clearScratchpad(void)
{
    memset(scratchpad, static_cast<uint8_t>(0x00), SCRATCHPAD_SIZE);
}

bool DS2430::writeMemory(const uint8_t* const source, const uint8_t length, const uint8_t position)
{
    for (uint8_t i = 0; i < length; ++i) {
        if ((position + i) >= sizeof(memory)) break;
        memory[position + i] = source[position + i];
    }

    return true;
}

bool DS2430::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}
