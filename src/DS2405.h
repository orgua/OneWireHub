// Single address switch @@@
// works, but reading back the value is not supported, because alarm search is not implemented yet
// this IC is not using standard protocol - it sends data after searchRom and alarmSearch
// native bus-features: alarm search

#ifndef ONEWIRE_DS2405_H
#define ONEWIRE_DS2405_H

#include "OneWireItem.h"

class DS2405 : public OneWireItem
{
private:

    bool pin_state;

public:

    static constexpr uint8_t family_code { 0x05 };

    DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void duty(OneWireHub * hub) final;

    void setPinState(const bool value)
    {
        pin_state = value;
    }

    bool getPinState(void) const
    {
        return pin_state;
    }
};

#endif
