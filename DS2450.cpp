#include "OneWireHub.h"
#include "DS2450.h"

DS2450::DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) :
        OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{

    memset(&memory, 0, sizeof(memory));
}

bool DS2450::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint16_t memory_address_start;
    uint8_t b;
    uint16_t crc;

    ow_crc16_reset();

    uint8_t done = hub->recv();
    switch (done)
    {
        case 0xAA: // READ MEMORY
            // Cmd
            ow_crc16_update(0xAA);

            // Adr1
            b = hub->recv();
            ((uint8_t *) &memory_address)[0] = b;
            ow_crc16_update(b);

            // Adr2
            b = hub->recv();
            ((uint8_t *) &memory_address)[1] = b;
            ow_crc16_update(b);

            memory_address_start = memory_address;

            for (int i = 0; i < 8; ++i)
            {
                b = memory[memory_address + i];
                hub->send(b);
            }

            crc = ow_crc16_get();
            hub->send(((uint8_t *) &crc)[0]);
            hub->send(((uint8_t *) &crc)[1]);

            if (dbg_sensor)
            {
                Serial.print("DS2450 : READ MEMORY : ");
                Serial.println(memory_address_start, HEX);
            }

            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2450=");
                Serial.println(done, HEX);
            }
            break;
    }

    return true;
}

bool DS2450::setPotentiometer(const uint16_t p1, const uint16_t p2, const uint16_t p3, const uint16_t p4)
{
    setPotentiometer(0, p1);
    setPotentiometer(1, p2);
    setPotentiometer(2, p3);
    setPotentiometer(3, p4);
    return true;
};

bool DS2450::setPotentiometer(const uint8_t number, const uint16_t value)
{
    if (number > 3) return 1;
    uint8_t lbyte = (value>>0) & static_cast<uint8_t>(0xFF);
    uint8_t hbyte = (value>>8) & static_cast<uint8_t>(0xFF);
    memory[2*number+0] = lbyte;
    memory[2*number+1] = hbyte;
    return true;
};
