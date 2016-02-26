#include "DS2433.h"

DS2433::DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (int i = 0; i < sizeof(memory); ++i)
        memory[i] = 0xFF;
}

bool DS2433::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint8_t  mem_offset;
    uint8_t  b;

    uint8_t cmd = hub->recv();

    switch (cmd)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            // Adr1
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;

            for (int i = 0; i < 32; ++i) // TODO: check for memory_address + 32 < sizeof()
            {
                hub->send(memory[memory_address + i]);
                if (hub->getError()) break;
            }

            break;

            // READ SCRATCHPAD COMMAND
        case 0xAA:
            // Adr1
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;

            // Offset
            mem_offset = hub->recv();

            break;

            // READ MEMORY
        case 0xF0:
            // Adr1
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;

            // data
            for (int i = 0; i < 32; ++i) // TODO: check for memory_address + 32 < sizeof()
                hub->send(memory[memory_address + i]);

            break;

        default:
            hub->raiseSlaveError(cmd);
            break;
    }
    return true;
}
