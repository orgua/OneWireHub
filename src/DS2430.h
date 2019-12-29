// 1Kb 1-Wire EEPROM
// works,
// note: datasheet is fuzzy, but device is similar to ds2433
// native bus-features: Overdrive capable

#ifndef ONEWIRE_DS2430_H
#define ONEWIRE_DS2430_H

#include "OneWireItem.h"

class DS2430 : public OneWireItem
{
private:

    static constexpr uint8_t  MEM_SIZE          { 32 };

    static constexpr uint8_t  SCRATCHPAD_SIZE   { 32 };
    static constexpr uint8_t  SCRATCHPAD_MASK   { 0b00011111 };

    static constexpr uint8_t  REG_ES_PF_MASK    { 0b00100000 }; // partial byte flag

    uint8_t memory[MEM_SIZE];

    uint8_t scratchpad[SCRATCHPAD_SIZE];

    uint8_t scratchpad_start_address;
    uint8_t scratchpad_size;

    void    clearScratchpad(void);

public:

    static constexpr uint8_t family_code        { 0x14 };

    DS2430(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    void    clearMemory(void);

    bool    writeMemory(const uint8_t* source, uint8_t length, uint8_t position = 0);
    bool    readMemory(uint8_t* destination, uint16_t length, uint16_t position = 0) const;
};

#endif
