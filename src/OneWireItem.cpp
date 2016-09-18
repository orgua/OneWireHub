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

void OneWireItem::sendID(OneWireHub *hub) {
    hub->send(ID, 8);
}

// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
// fast but needs more storage:
//  https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp --> calc with table (EOF)


// INFO: this is the slow but memory saving version of the CRC() --> the calculation is not time-critical and happens offline
// alternative for AVR: http://www.atmel.com/webdoc/AVRLibcReferenceManual/group__util__crc_1ga37b2f691ebbd917e36e40b096f78d996.html

uint8_t OneWireItem::crc8(const uint8_t address[], const uint8_t length, const uint8_t init = 0)
{
    uint8_t crc = init;

    for (uint8_t i = 0; i < length; ++i)
    {
#if defined(__AVR__)
        crc = _crc_ibutton_update(crc, address[i]);
#else
        uint8_t inByte = address[i];
        for (uint8_t j = 8; j; --j)
        {
            uint8_t mix = (crc ^ inByte) & static_cast<uint8_t>(0x01);
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inByte >>= 1;
        }
#endif
    }
    return crc;
};


uint16_t OneWireItem::crc16(const uint8_t address[], const uint8_t length, const uint16_t init = 0)
{
    uint16_t crc = init; // init value

#if defined(__AVR__)
    for (uint8_t i = 0; i < length; ++i)
    {
        crc = _crc16_update(crc, address[i]);
    }
#else
    static const uint8_t oddParity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    for (uint8_t i = 0; i < length; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = address[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddParity[cdata & 0x0F] ^ oddParity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }
#endif
    return crc;
};

uint16_t OneWireItem::crc16(uint8_t value, uint16_t crc)
{
#if defined(__AVR__)
    return _crc16_update(crc, value);
#else
    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
#endif
};