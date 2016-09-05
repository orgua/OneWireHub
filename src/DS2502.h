// 0x28  1Kb 1-Wire EEPROM, Add Only Memory
// not finished
// Copyright by Kondi, https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave

#ifndef ONEWIRE_DS2502_H
#define ONEWIRE_DS2502_H

#include "OneWireItem.h"

class DS2502 : public OneWireItem
{
private:


public:
    static constexpr uint8_t family_code = 0x09;

    uint8_t memory[128]; // should be private, but it is easier to modify

    DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);
};

#endif