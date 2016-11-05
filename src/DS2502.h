// 0x09  1Kbit 1-Wire EEPROM, Add Only Memory
// works, writing could not be tested (DS9490 does not support hi-voltage mode and complains)
// Copyright by Kondi (initial version), https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
// DS2501: 0x11, autoset to 512bits
// dell powersupply: 0x28

#ifndef ONEWIRE_DS2502_H
#define ONEWIRE_DS2502_H

#include "OneWireItem.h"

class DS2502 : public OneWireItem
{
private:

    static constexpr uint8_t    PAGE_COUNT          { 4 };
    static constexpr uint8_t    PAGE_SIZE           { 32 }; // bytes
    static constexpr uint8_t    PAGE_MASK           { PAGE_SIZE - 1 };

    static constexpr uint8_t    MEM_SIZE            { PAGE_COUNT * PAGE_SIZE }; // bytes
    static constexpr uint16_t   MEM_MASK            { MEM_SIZE - 1 };

    static constexpr uint8_t    STATUS_SIZE         { 8 };

    static constexpr uint8_t    STATUS_WP_PAGES     {0x00}; // 1 byte -> Page write protection and page used status
    static constexpr uint8_t    STATUS_PG_REDIR     {0x01}; // 4 byte -> Page redirection
    static constexpr uint8_t    STATUS_UNDEF_B1     {0x05}; // 2 byte -> reserved / undefined
    static constexpr uint8_t    STATUS_FACTORYP     {0x07}; // 2 byte -> factoryprogrammed 0x00

    uint8_t  memory[MEM_SIZE];    // 4 pages of 32 bytes
    uint8_t  status[STATUS_SIZE]; // eprom status bytes:
    uint8_t  sizeof_memory;       // device specific "real" size

    uint8_t  translateRedirection(const uint8_t source_address);

public:

    static constexpr uint8_t family_code = 0x09; // the ds2502

    DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub *hub);

    void    clearMemory(void);
    void    clearStatus(void);

    bool    writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position = 0);
    bool    readMemory(uint8_t * const destination, const uint8_t length, const uint8_t position = 0);

    uint8_t readStatus(const uint8_t address);
    uint8_t writeStatus(const uint8_t address, const uint8_t value);

    void    setPageProtection(const uint8_t page);
    bool    getPageProtection(const uint8_t page);

    void    setPageUsed(const uint8_t page);
    bool    getPageUsed(const uint8_t page);

    bool    setPageRedirection(const uint8_t page_source, const uint8_t page_destin);
    uint8_t getPageRedirection(const uint8_t page);
};

#endif
