//  DS2502 1Kb EEPROM in genuine DELL AC adapter

#ifndef ONEWIRE_DELLAC_H
#define ONEWIRE_DELLAC_H

#include "OneWireItem.h"

class DellAC : public OneWireItem
{
private:
    uint8_t chargerData[4] = {0xFB, 0x31, 0x33, 0x30};//130W
    //uint8_t chargerData[4] = {0xFB, 0x30, 0x39, 0x30};//90W
    //uint8_t chargerData[4] = {0xFB, 0x30, 0x36, 0x36};//65W

public:
    static constexpr uint8_t family_code = 0x23;
    DellAC(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);
    void duty(OneWireHub * hub) final;
};

#endif
