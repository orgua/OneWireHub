// 0x01  Serial Number
// Works - 100%

#ifndef ONEWIRE_DS2401_H
#define ONEWIRE_DS2401_H

#include "OneWireItem.h"

class DS2401 : public OneWireItem
{
private:

public:
    static constexpr uint8_t family_code = 0x01;

    DS2401(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);
};

#endif