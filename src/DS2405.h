// 0x05  Single address switch @@@
// works

#ifndef ONEWIRE_DS2405_H
#define ONEWIRE_DS2405_H

#include "OneWireItem.h"

class DS2405 : public OneWireItem
{
private:
    bool pin_state;

public:
    static constexpr uint8_t family_code = 0x05;

    DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    bool readState(void)
    {
        return pin_state;
    };
    void setState(const bool value)
    {
        pin_state = value;
    };
};

#endif