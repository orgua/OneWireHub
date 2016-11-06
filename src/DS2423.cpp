#include "DS2423.h"

DS2423::DS2423(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    // empty
};

void DS2423::duty(OneWireHub * const hub)
{
    constexpr uint8_t DUMMY_xFF = 0xFF;
    constexpr uint8_t DUMMY_x00 = 0x00;
    uint16_t reg_TA, crc = 0;  // target_address
    //uint16_t memory_address_start; // not used atm, but maybe later
    //uint8_t b;

    uint8_t cmd;
    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0xA5:      // Read Memory + Counter
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) return;
            //memory_address_start = ta;

            // data
            for (int8_t i = 0; i < 32; ++i) // TODO: check for (ta + 32) < sizeof() before running out of allowed range
            {
                if (hub->send(&DUMMY_xFF,1,crc)) return;
            }

            // 4x cnt & 4x zero
            for (uint8_t i = 0; i < 8; ++i)
            {
                if (hub->send(&DUMMY_x00,1,crc)) return;
            }

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
};
