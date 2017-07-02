// Dual channel addressable switch
// Works, master can latch the pin and pull it thereby down
// native bus-features: Overdrive capable

#ifndef ONEWIRE_DS2413_H
#define ONEWIRE_DS2413_H

#include "OneWireItem.h"

class DS2413 : public OneWireItem
{
private:

    bool pin_state[2];  // sensed input for A and B
    bool pin_latch[2];  // PIO can be set to input (0) or output-to-zero (1)

public:

    static constexpr uint8_t family_code { 0x3A };

    DS2413(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    bool    setPinState(const uint8_t a_or_b, const bool value)
    {
        if (value && pin_latch[a_or_b & 1]) return false; // can't set 1 because pin is latched
        pin_state[a_or_b & 1] = value;
        return true;
    }

    bool    getPinState(const uint8_t a_or_b) const
    {
        return pin_state[a_or_b & 1];
    }

    void    setPinLatch(const uint8_t a_or_b, const bool value) // latching a pin will pull it down (state=zero)
    {
        pin_latch[a_or_b & 1] = value;
        if (value) setPinState(a_or_b, false);
    }

    bool    getPinLatch(const uint8_t a_or_b) const
    {
        return pin_latch[a_or_b & 1];
    }
};

#endif
