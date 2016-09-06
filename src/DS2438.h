// 0x26  Smart Battery Monitor
// works

#ifndef ONEWIRE_DS2438_H
#define ONEWIRE_DS2438_H

#include "OneWireItem.h"

static constexpr uint8_t MemDS2438[64] =
        {
                //  memory[0] = DS2438_IAD | DS2438_CA | DS2438_EE | DS2438_AD;
                0x09, 0x20, 0x14, 0xAC, 0x00, 0x40, 0x01, 0x00,
                0x5A, 0xC8, 0x05, 0x02, 0xFF, 0x08, 0x00, 0xFC,
                0x00, 0x00, 0x00, 0x00, 0x6D, 0x83, 0x03, 0x02,
                0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x26, 0xBF, 0x8E, 0x30, 0x01, 0x00, 0x00, 0x00,
                0x2D, 0x07, 0xDB, 0x15, 0x00, 0x00, 0x00, 0x00,
                0x28, 0x80, 0xDC, 0x1B, 0x03, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        };

struct DS2438_page0 // overlay with memory if needed (like done in ds2408)
{
    uint8_t flags;
    int16_t temp;
    int16_t volt;
    int16_t curr;
    uint8_t threshold;
};



class DS2438 : public OneWireItem
{
private:
    static constexpr uint8_t PAGE_EMU_COUNT = 8; // how much of the real 8 pages should be emulated, use at least 1, max 8

    // Register Addresses
    static constexpr uint8_t DS2438_IAD  = 0x01; // enable automatic current measurements
    static constexpr uint8_t DS2438_CA   = 0x02; // enable current accumulator (page7, byte 4-7)
    static constexpr uint8_t DS2438_EE   = 0x04; // shadow accu to eeprom
    static constexpr uint8_t DS2438_AD   = 0x08; // 1: battery voltage, 0: ADC-GPIO
    static constexpr uint8_t DS2438_TB   = 0x10; // temperature busy flag
    static constexpr uint8_t DS2438_NVB  = 0x20; // eeprom busy flag
    static constexpr uint8_t DS2438_ADB  = 0x40; // adc busy flag

    uint8_t memory[(PAGE_EMU_COUNT+1)*8]; // there are another 8byte for garbage-collection if master chooses out of bound adress
    uint8_t crc[(PAGE_EMU_COUNT+1)];      // keep the matching crc for each memory-page, reading can be very timesensitive

    void calcCRC(const uint8_t page);

public:
    static constexpr uint8_t family_code = 0x26;

    DS2438(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    void setTemp(const float   temp_degC);
    void setTemp(const int8_t temp_degC);

    void setVolt(const uint16_t voltage_10mV);

    void setCurr(const int16_t value);
};

#endif