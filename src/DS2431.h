// 1Kb 1-Wire EEPROM
// works,
// note: datasheet is fuzzy, but device is similar to ds2433
// native bus-features: Overdrive capable

#ifndef ONEWIRE_DS2431_H
#define ONEWIRE_DS2431_H

#include "OneWireItem.h"

class DS2431 : public OneWireItem
{
private:

    static constexpr uint8_t  MEM_SIZE          { 144 };

    static constexpr uint8_t  PAGE_SIZE         { 32 };
    static constexpr uint8_t  PAGE_COUNT        { MEM_SIZE / PAGE_SIZE };
    static constexpr uint8_t  PAGE_MASK         { 0b00011111 };

    static constexpr uint8_t  SCRATCHPAD_SIZE   { 8 };
    static constexpr uint8_t  SCRATCHPAD_MASK   { 0b00000111 };

    static constexpr uint8_t  REG_ES_PF_MASK    { 0b00100000 }; // partial byte flag
    static constexpr uint8_t  REG_ES_ZERO_MASK  { 0b01011000 }; // reads always zero
    static constexpr uint8_t  REG_ES_AA_MASK    { 0b10000000 }; // authorization accepted (data copied to target memory)

    static constexpr uint8_t  WP_MODE           { 0x55 }; // write protect mode
    static constexpr uint8_t  EP_MODE           { 0xAA }; // eprom mode

    uint8_t memory[MEM_SIZE];

    uint8_t scratchpad[SCRATCHPAD_SIZE];
    uint8_t page_protection;
    uint8_t page_eprom_mode;

    bool    updatePageStatus(void);
    void    clearScratchpad(void);

public:

    static constexpr uint8_t family_code        { 0x2D };

    DS2431(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    void    clearMemory(void);

    bool    writeMemory(const uint8_t* source, uint8_t length, uint8_t position = 0);
    bool    readMemory(uint8_t* destination, uint16_t length, uint16_t position = 0) const;

    void    setPageProtection(uint8_t position);
    bool    getPageProtection(uint8_t position) const;

    void    setPageEpromMode(uint8_t position);
    bool    getPageEpromMode(uint8_t position) const;
};

#endif
