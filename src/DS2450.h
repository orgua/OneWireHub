// 4 channel A/D
// works, but without alarm features and other controllable functions beside ADC-Reading
// native bus-features: Overdrive capable, alarm search

#ifndef ONEWIRE_DS2450_H
#define ONEWIRE_DS2450_H

#include "OneWireItem.h"

class DS2450 : public OneWireItem
{
private:

    static constexpr uint8_t POTI_COUNT  { 4 };
    static constexpr uint8_t PAGE_COUNT  { 4 };
    static constexpr uint8_t PAGE_SIZE   { 2*POTI_COUNT };
    static constexpr uint8_t PAGE_MASK   { 0b00000111 };

    static constexpr uint8_t MEM_SIZE    { PAGE_COUNT*PAGE_SIZE };

    uint8_t memory[MEM_SIZE];
    // Page1 : conversion results:  16 bit for Channel A, B, C & D, power on default: 0x00
    // Page2 : control / status:    16 bit per channel
    // Page3 : alarm settings:      16 bit per channel
    // Page3 : factory calibration

    void correctMemory(void);

public:
    static constexpr uint8_t family_code { 0x20 };

    DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void     duty(OneWireHub * hub) final;

    void     clearMemory(void);

    bool     setPotentiometer(uint16_t p1, uint16_t p2, uint16_t p3, uint16_t p4);
    bool     setPotentiometer(uint8_t channel, uint16_t value);
    uint16_t getPotentiometer(uint8_t channel) const;
};

#endif
