// 0x23  4Kb 1-Wire EEPROM

#ifndef ONEWIRE_DS2433_H
#define ONEWIRE_DS2433_H

#include "OneWireItem.h"

class DS2433 : public OneWireItem
{
private:
    uint8_t memory[512];

public:
    static constexpr uint8_t family_code = 0x23;

    DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);
};

#endif