#include "DS2423.h"

DS2423::DS2423(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

bool DS2423::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint16_t memory_address_start;
    uint8_t b;
    uint16_t crc = 0;

    uint8_t cmd = hub->recvAndCRC16(crc);

    switch (cmd)
    {
        // Read Memory + Counter
        case 0xA5:

            reinterpret_cast<uint8_t *>(&memory_address)[0] = hub->recvAndCRC16(crc); // Adr1
            reinterpret_cast<uint8_t *>(&memory_address)[1] = hub->recvAndCRC16(crc);

            memory_address_start = memory_address;

            // data
            for (int i = 0; i < 32; ++i) // TODO: check for (memory_address + 32) < sizeof() before running out of allowed range
            {
                crc = hub->sendAndCRC16(0xff,crc);
            }

            // cnt
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);

            // zero
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);
            crc = hub->sendAndCRC16(0x00,crc);

            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);

            break;

        default:
            hub->raiseSlaveError(cmd);
            break;
    }

    return true;
}

