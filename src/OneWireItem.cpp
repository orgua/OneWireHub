#include "OneWireItem.h"

OneWireItem::OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7)
{
    ID[0] = ID1;
    ID[1] = ID2;
    ID[2] = ID3;
    ID[3] = ID4;
    ID[4] = ID5;
    ID[5] = ID6;
    ID[6] = ID7;
    ID[7] = crc8(ID, 7);
};


// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
// fast but needs more storage:
//  https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp --> calc with table (EOF)


// INFO: this is the slow but memory saving version of the CRC() --> the calculation is not timecritical and happens offline
uint8_t OneWireItem::crc8(const uint8_t addr[], const uint8_t len)
{
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; ++i)
    {
        uint8_t inbyte = addr[i];
        for (uint8_t j = 8; j; --j)
        {
            uint8_t mix = (crc ^ inbyte) & static_cast<uint8_t>(0x01);
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

uint16_t OneWireItem::crc16(const uint8_t addr[], const uint8_t len)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    uint16_t crc = 0; // initvalue

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = addr[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
};
