// 0x28  1Kb 1-Wire EEPROM, Add Only Memory
// not finished
// Copyright by Kondi (initial version), https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave

#ifndef ONEWIRE_DS2502_H
#define ONEWIRE_DS2502_H

#include "OneWireItem.h"

class DS2502 : public OneWireItem
{
private:
    uint8_t memory[128]; // 4 pages of 32 bytes
    uint8_t scratchpad[8];
    uint8_t status[8]; // eprom status bytes

public:
    static constexpr uint8_t family_code = 0x09;

    DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    void clearMemory(void);

    bool writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position = 0);
};

#endif