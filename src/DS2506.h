// (0x0F) 64kbit EEPROM, Add Only Memory

#ifndef ONEWIRE_DS2506_H
#define ONEWIRE_DS2506_H

#include "OneWireItem.h"

class DS2506 : public OneWireItem
{
private:

    static constexpr uint8_t PAGE_COUNT  = 4;
    static constexpr uint8_t PAGE_SIZE   = 32;
    static constexpr uint8_t PAGE_MASK   = 0b00011111;
    static constexpr uint8_t SIZE_MEM    = PAGE_COUNT * PAGE_SIZE;

    uint8_t     memory[SIZE_MEM]; // 4 pages of 32 bytes
    uint16_t    sizeof_memory;
    uint8_t     status[8]; // eprom status bytes

    void    clearStatus(void);
    bool    checkProtection(const uint16_t reg_address = 0);
    uint8_t translateRedirection(const uint16_t reg_address = 0);

public:
    static constexpr uint8_t family_code = 0x13;

    // Initializer will get stuck in an infinite loop when you choose a sensor
    DS2506(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void duty(OneWireHub *hub);

    void clearMemory(void);

    bool writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position = 0);

    bool redirectPage(const uint8_t page_source, const uint8_t page_dest);
    bool protectPage(const uint8_t page, const bool status_protected);
};

#endif
