#ifndef ONEWIREHUB_ONEWIREITEM_H
#define ONEWIREHUB_ONEWIREITEM_H

#include "OneWireHub.h"

#if defined(__AVR__)
#include <util/crc16.h>
#endif

// Feature to get first byte (family code) constant for every sensor --> var4 is implemented
// - var 1: use second init with one byte less (Serial 1-6 instead of ID)
// - var 2: write ID1 of OneWireItem with the proper value without asking
// - var 3: rewrite the OneWireItem-Class and implement something like setFamilyCode()
// - var 4: make public family_code in sensor mandatory and just put it into init() if wanted --> prefer this

class OneWireItem
{
public:

    OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    ~OneWireItem() = default; // TODO: detach if deleted before hub

    OneWireItem(const OneWireItem& owItem) = delete;             // disallow copy constructor
    OneWireItem(OneWireItem&& owItem) = default;               // default  move constructor
    OneWireItem& operator=(OneWireItem& owItem) = delete;        // disallow copy assignment
    OneWireItem& operator=(const OneWireItem& owItem) = delete;  // disallow copy assignment
    OneWireItem& operator=(OneWireItem&& owItem) = delete;       // disallow move assignment

    uint8_t ID[8];

    void sendID(OneWireHub * hub) const;

    virtual void duty(OneWireHub * hub) = 0;

    static uint8_t crc8(const uint8_t data[], uint8_t data_size, uint8_t crc_init = 0);

    // takes ~(5.1-7.0)µs/byte (Atmega328P@16MHz) depends from address_size (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(const uint8_t address[], uint8_t len, uint16_t init = 0);

    // CRC16 of type 0xA001 for little endian
    // takes ~6µs/byte (Atmega328P@16MHz) (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(uint8_t value, uint16_t crc);

};


#endif //ONEWIREHUB_ONEWIREITEM_H
