#include "OneWireHub.h"
#include "DS2423.h"

DS2423::DS2423(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

bool DS2423::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint16_t memory_address_start;
    uint8_t b;
    uint16_t crc;

    ow_crc16_reset();

    uint8_t done = hub->recv();
    switch (done)
    {
        // Read Memory + Counter
        case 0xA5:
            ow_crc16_update(0xA5);

            // Adr1
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;
            ow_crc16_update(b);

            // Adr2
            b = hub->recv();
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;
            ow_crc16_update(b);

            memory_address_start = memory_address;

            // data
            for (int i = 0; i < 32; ++i) // TODO: check for memory_address + 32 < sizeof()
            {
                hub->send(0xff);
                ow_crc16_update(0xff);
            }

            // cnt
            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            // zero
            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            hub->send(0x00);
            ow_crc16_update(0x00);

            // crc
            crc = ow_crc16_get();
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            ow_crc16_reset();

            if (dbg_sensor)
            {
                Serial.print("DS2423 : Read Memory + Counter : ");
                Serial.println(memory_address_start, HEX);
            }

            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2423=");
                Serial.println(done, HEX);
            }
            break;
    }

    return true;
}

