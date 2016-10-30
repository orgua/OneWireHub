// 0x29  8-Channel Addressable Switch @@@
// basic operation works

#ifndef ONEWIRE_DS2408_H
#define ONEWIRE_DS2408_H

#include "OneWireItem.h"

class DS2408 : public OneWireItem
{
private:

    // Register Indexes
    static constexpr uint8_t  DS2408_OFFSET                 = 0x88;
    static constexpr uint8_t  DS2408_MEMSIZE                = 8;

    static constexpr uint8_t  DS2408_PIO_LOGIC_REG          = 0; // 0x88 - Current state
    static constexpr uint8_t  DS2408_PIO_OUTPUT_REG         = 1; // 0x89 - Last write, latch state register
    static constexpr uint8_t  DS2408_PIO_ACTIVITY_REG       = 2; // 0x8A - State Change Activity
    static constexpr uint8_t  DS2408_SEARCH_MASK_REG        = 3; // 0x8B - RW Conditional Search Channel Selection Mask
    static constexpr uint8_t  DS2408_SEARCH_SELECT_REG      = 4; // 0x8C - RW Conditional Search Channel Polarity Selection
    static constexpr uint8_t  DS2408_CONTROL_STATUS_REG     = 5; // 0x8D - RW Control Register
    static constexpr uint8_t  DS2408_RD_ABOVE_ALWAYS_FF_8E  = 6; // 0x8E - these bytes give always 0xFF
    static constexpr uint8_t  DS2408_RD_ABOVE_ALWAYS_FF_8F  = 7; // 0x8F - these bytes give always 0xFF

    uint8_t memory[DS2408_MEMSIZE];

public:
    static constexpr uint8_t family_code = 0x29;

    DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void duty(OneWireHub *hub);

    bool getPinState(uint8_t pinNumber);
    uint8_t getPinStates(void);
    void setPinState(uint8_t pinNumber, bool value);
};

#endif
