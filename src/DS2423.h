// 4kb 1-Wire RAM with Counter
// works
// native bus-features: Overdrive capable

#ifndef ONEWIRE_DS2423_H
#define ONEWIRE_DS2423_H

#include "OneWireItem.h"

class DS2423 : public OneWireItem
{
private:

    static constexpr uint16_t MEM_SIZE          { 512 };

    static constexpr uint8_t  PAGE_SIZE         { 32 };
    static constexpr uint16_t PAGE_COUNT        { MEM_SIZE / PAGE_SIZE };
    static constexpr uint8_t  PAGE_MASK         { 0b00011111 };

    static constexpr uint8_t  COUNTER_COUNT     { 4 };
    static constexpr uint8_t  COUNTER_PAGE_START{ 12 }; // page 12&13 have write-counters, page 14&15 transmit the hw-counters

    static constexpr uint8_t  REG_ES_PF_MASK    { 0b00100000 }; // partial byte flag
    static constexpr uint8_t  REG_ES_ZERO_MASK  { 0b01000000 }; // reads always zero
    static constexpr uint8_t  REG_ES_AA_MASK    { 0b10000000 }; // authorization accepted (data copied to target memory)

    static constexpr uint16_t REG_TA_MASK       { 0x01FF }; // Adresses will be stripped of the highest 7 bytes

    uint8_t     memory[MEM_SIZE]; // 4kbit max storage
    uint8_t     scratchpad[PAGE_SIZE];
    uint32_t    memcounter[COUNTER_COUNT];

    void    clearScratchpad(void);

public:

    static constexpr uint8_t family_code        { 0x1D };

    DS2423(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void     duty(OneWireHub * hub) final;

    void     clearMemory(void);

    bool     writeMemory(const uint8_t* source, uint16_t length, uint16_t position = 0);
    bool     readMemory(uint8_t* destination, uint16_t length, uint16_t position = 0) const;

    void     setCounter(uint8_t counter, uint32_t value);
    uint32_t getCounter(uint8_t counter);
    void     incrementCounter(uint8_t counter);
    void     decrementCounter(uint8_t counter);

};

#endif
