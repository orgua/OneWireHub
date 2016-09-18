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
    static constexpr uint8_t page_mask = 0b00011111;

    uint8_t memory[128]; // 4 pages of 32 bytes
    uint8_t sizeof_memory;
    uint8_t status[8]; // eprom status bytes

    void    clearStatus(void);
    bool    checkProtection(const uint16_t reg_address = 0);
    uint8_t translateRedirection(const uint16_t reg_address = 0);

public:
    static constexpr uint8_t family_code = 0x09;

    DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    void clearMemory(void);

    bool writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position = 0);

    bool redirectPage(const uint8_t page_source, const uint8_t page_dest);
    bool protectPage(const uint8_t page, const bool status_protected);
};

#endif