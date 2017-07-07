// 8-Channel Addressable Switch @@@
// works, but no alarm search and higher logic / output / control register-action
// native bus-features: Overdrive capable, alarm search

#ifndef ONEWIRE_DS2408_H
#define ONEWIRE_DS2408_H

#include "OneWireItem.h"

class DS2408 : public OneWireItem
{
private:

    static constexpr uint8_t  MEM_SIZE                  { 8 };

    // Register Indexes
    static constexpr uint8_t  REG_OFFSET                { 0x88 };
    static constexpr uint8_t  REG_PIO_LOGIC             { 0 }; // 0x88 - Current state
    static constexpr uint8_t  REG_PIO_OUTPUT            { 1 }; // 0x89 - Last write, latch state register
    static constexpr uint8_t  REG_PIO_ACTIVITY          { 2 }; // 0x8A - State Change Activity
    static constexpr uint8_t  REG_SEARCH_MASK           { 3 }; // 0x8B - RW Conditional Search Channel Selection Mask
    static constexpr uint8_t  REG_SEARCH_SELECT         { 4 }; // 0x8C - RW Conditional Search Channel Polarity Selection
    static constexpr uint8_t  REG_CONTROL_STATUS        { 5 }; // 0x8D - RW Control Register
    static constexpr uint8_t  REG_RD_ABOVE_ALWAYS_FF_8E { 6 }; // 0x8E - these bytes give always 0xFF
    static constexpr uint8_t  REG_RD_ABOVE_ALWAYS_FF_8F { 7 }; // 0x8F - these bytes give always 0xFF

    uint8_t memory[MEM_SIZE];

public:

    static constexpr uint8_t family_code                { 0x29 };

    DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    void    clearMemory(void);

    void    setPinState(uint8_t pinNumber, bool value);
    bool    getPinState(uint8_t pinNumber) const;
    uint8_t getPinState(void) const;

    void    setPinActivity(uint8_t pinNumber, bool value);
    bool    getPinActivity(uint8_t pinNumber) const;
    uint8_t getPinActivity(void) const;

    // TODO: do we need FN to set the output register?

};

#endif
