// (0x0F) 64kbit EEPROM, Add Only Memory

#ifndef ONEWIRE_DS2506_H
#define ONEWIRE_DS2506_H

#include "OneWireItem.h"

class DS2506 : public OneWireItem
{
private:

    // Problem: atmega has 2kb RAM, this IC offers 8kb
    // Solution: out-of-bound-memory will be mapped to the last page available
    static constexpr uint16_t MEM_SIZE_PROPOSE  = 256; // TUNE HERE! Give this device as much RAM as your CPU can spare

    static constexpr uint8_t  PAGE_SIZE         = 32;
    static constexpr uint16_t PAGE_COUNT        = MEM_SIZE_PROPOSE / PAGE_SIZE;
    static constexpr uint8_t  PAGE_MASK         = 0b00011111;

    static constexpr uint16_t MEM_SIZE          = PAGE_COUNT * PAGE_SIZE;
    static constexpr uint16_t MEM_MASK          = MEM_SIZE - 1;

    static constexpr uint16_t STATUS_SEGMENT    = (PAGE_COUNT/8);
    static constexpr uint16_t STATUS_SIZE       = PAGE_COUNT + (3*STATUS_SEGMENT);

    static_assert(MEM_SIZE > 255, "REAL MEM SIZE IS TOO SMALL");
    static_assert(STATUS_SEGMENT > 0, "REAL MEM SIZE IS TOO SMALL");

    uint8_t     memory[MEM_SIZE]; // 4 pages of 32 bytes
    uint16_t    sizeof_memory;
    uint16_t    sizeof_status;
    uint16_t    page_count, status_segment;
    uint8_t     status[STATUS_SIZE]; // eprom status bytes
    // PAGE_COUNT/8 bytes -> Page write protection
    // PAGE_COUNT/8 bytes -> Redirection write protection
    // PAGE_COUNT/8 bytes -> Page used status (written one)
    // PAGE_COUNT bytes   -> Redirection to page, 0xFF if valid, ones complement (xFD is page 2)

    void     clearStatus(void);
    bool     checkProtection(const uint16_t reg_address = 0);

    uint16_t translateRedirection(const uint16_t reg_address = 0); // react to redirection in status and not available memory
    uint16_t translateStatusAddress(const uint16_t reg_address);   // plan to only use a subset of the real memory
    uint8_t  getRedirection(const uint16_t reg_address);

public:
    static constexpr uint8_t family_code = 0x0F;

    DS2506(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void duty(OneWireHub *hub);

    void clearMemory(void);

    bool writeMemory(const uint8_t* source, const uint16_t length, const uint16_t position = 0);
    bool readMemory(uint8_t* destination, const uint16_t length, const uint16_t position);

    bool redirectPage(const uint8_t page_source, const uint8_t page_dest);
    bool protectPage(const uint8_t page, const bool status_protected);
};

#endif
