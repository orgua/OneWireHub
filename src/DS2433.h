// 0x23  4Kb 1-Wire EEPROM
// works

#ifndef ONEWIRE_DS2433_H
#define ONEWIRE_DS2433_H

#include "OneWireItem.h"

class DS2433 : public OneWireItem
{
private:
    uint8_t memory[512]; // 4kb max storage

public:
    static constexpr uint8_t family_code = 0x23;

    DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    void clearMemory(void);
    bool writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position = 0);
};

#endif