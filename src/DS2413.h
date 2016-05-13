// 0x3A  Dual channel addressable switch
// Works - 100%, supported overdrive not implemented

#ifndef ONEWIRE_DS2413_H
#define ONEWIRE_DS2413_H

#include "OneWireItem.h"

class DS2413 : public OneWireItem
{
private:

    bool pin_state[2];  // sensed input for A and B
    bool pin_latch[2];  // PIO can be set to input (0) or output-to-zero (1)

public:
    static constexpr uint8_t family_code = 0x3A;

    DS2413(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    bool readState(const uint8_t a_or_b)
    {
        return pin_state[a_or_b & 1];
    };

    bool setState(const uint8_t a_or_b, const bool value)
    {
        if (value && pin_latch[a_or_b & 1])
            return 0; // can't set 1 because pin is latched
        pin_state[a_or_b & 1] = value;
        return 1;
    };

    bool readLatch(const uint8_t a_or_b)
    {
        return pin_latch[a_or_b & 1];
    };

    void setLatch(const uint8_t a_or_b, const bool value)
    {
        pin_latch[a_or_b & 1] = value;
        if (value) setState(a_or_b, 0);
    };
};

#endif