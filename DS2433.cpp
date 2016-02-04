#include "OneWireHub.h"
#include "DS2433.h"

#define DEBUG_DS2433

DS2433::DS2433(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (int i = 0; i < sizeof(this->memory); i++)
        this->memory[i] = 0xFF;
}

bool DS2433::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint8_t mem_offset;
    uint8_t b;
    uint16_t crc;

    uint8_t done = hub->recv();

    switch (done)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            // Adr1
            b = hub->recv();
            ((uint8_t *) &memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            ((uint8_t *) &memory_address)[1] = b;

            for (int i = 0; i < 32; i++)
            {
                hub->send(this->memory[memory_address + i]);
                if (hub->errno) break;
            }

#ifdef DEBUG_DS2433
            Serial.print("DS2433 : WRITE SCRATCHPAD COMMAND : ");
            Serial.println(memory_address, HEX);
#endif

            break;

            // READ SCRATCHPAD COMMAND
        case 0xAA:
            // Adr1
            b = hub->recv();
            ((uint8_t *) &memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            ((uint8_t *) &memory_address)[1] = b;

            // Offset
            mem_offset = hub->recv();

#ifdef DEBUG_DS2433
            Serial.print("DS2433 : READ SCRATCHPAD COMMAND : ");
            Serial.print(memory_address, HEX);
            Serial.print(",");
            Serial.println(mem_offset, HEX);
#endif

            break;

            // READ MEMORY
        case 0xF0:
            // Adr1
            b = hub->recv();
            ((uint8_t *) &memory_address)[0] = b;

            // Adr2
            b = hub->recv();
            ((uint8_t *) &memory_address)[1] = b;

            // data
            for (int i = 0; i < 32; i++)
                hub->send(this->memory[memory_address + i]);

#ifdef DEBUG_DS2433
            Serial.print("DS2433 : READ MEMORY : ");
            Serial.println(memory_address, HEX);
#endif

            break;

        default:
#ifdef DEBUG_hint
            Serial.print("DS2433=");
            Serial.println(done, HEX);
#endif
            break;
    }
}
