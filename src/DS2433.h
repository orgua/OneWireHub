// 0x23  4Kb 1-Wire EEPROM
// works

#ifndef ONEWIRE_DS2433_H
#define ONEWIRE_DS2433_H

#include "OneWireItem.h"

class DS2433 : public OneWireItem
{
private:

    static constexpr uint16_t MEM_SIZE          = 512;

    static constexpr uint8_t  PAGE_SIZE         = 32;
    static constexpr uint16_t PAGE_COUNT        = MEM_SIZE / PAGE_SIZE;
    static constexpr uint8_t  PAGE_MASK         = 0b00011111;

    uint8_t memory[MEM_SIZE]; // 4kbit max storage

public:

    static constexpr uint8_t family_code = 0x23;

    DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * const hub);

    void    clearMemory(void);

    bool    writeMemory(const uint8_t* const source, const uint16_t length, const uint16_t position = 0);
    bool    readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position = 0) const;
};

#endif
