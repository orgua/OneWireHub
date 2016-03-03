#ifndef ONEWIREHUB_ONEWIREITEM_H
#define ONEWIREHUB_ONEWIREITEM_H

#include "OneWireHub.h"

// Feature to get first byte (family code) constant for every sensor --> var4 is implemented
// - var 1: use second init with one byte less (Serial 1-6 instead of ID)
// - var 2: write ID1 of OneWireItem with the proper value without asking
// - var 3: rewrite the OneWireItem-Class and implement something like setFamilyCode()
// - var 4: make public family_code in sensor mandatory and just put it into init() if wanted --> prefer this

class OneWireItem
{
public:

    OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t ID[8];

    virtual bool duty(OneWireHub *hub) = 0;

    static uint8_t crc8(const uint8_t address[], const uint8_t len);

    // takes ~(5.1-7.0)µs/byte (Atmega328P@16MHz) depends from address_size (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(const uint8_t address[], const uint8_t len);

    // CRC16 of type 0xA001 for little endian
    // takes ~6µs/byte (Atmega328P@16MHz) (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(uint8_t value, uint16_t crc) // TODO: further tuning with asm
    {
        static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
        value = (value ^ static_cast<uint8_t>(crc));
        crc >>= 8;
        if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
        uint16_t cdata = (static_cast<uint16_t>(value) << 6);
        crc ^= cdata;
        crc ^= (static_cast<uint16_t>(cdata) << 1);
        return crc;
    };

};


#endif //ONEWIREHUB_ONEWIREITEM_H
