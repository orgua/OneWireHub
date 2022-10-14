// 256 bit EEPROM / NV-RAM
// 256 bit Scratchpad
// Cycle Counter and temperature, status and ID register
// TODO: this should be the more powerful DS2436

#ifndef ONEWIREHUB_DS2434_H
#define ONEWIREHUB_DS2434_H

#include "OneWireItem.h"

class DS2434 : public OneWireItem
        {
private:
/*
  PAGE 1 NV1:
    - lockable nonvolatile, 24 byte
    - user data: battery chemistry descriptors, manufacturing lot codes, gas gauge information ..
  PAGE 2 NV2:
    - nonvolatile, 8 byte
  PAGE 3 SRAM:
    - volatile! lost if battery goes flat
    - gas gauge & self-discharge info
  PAGE 4 SRAM:
    - B0: Temp from 0 to 127 deg C in 1/2 deg steps (u8)
    - B1: Temp from -40 to + 85 deg C in 1 deg steps (i8)
    - B2: Status0, init = 0xF8
        - bit0 Temperature busy
        - bit1 nonvolatile mem busy
        - bit2 NV1 is locked
    - B3: Status1, init = 0xFF
  PAGE 5 SRAM:
    - B0:1: ID, 16 bit ROM, 0x5344
    - B2:3: cycle counter
 */
    static constexpr uint8_t  PAGE1_SIZE         { 24 }; // NV1, lockable nonvolatile
    static constexpr uint8_t  PAGE1_ADDR         { 0x00 };
    static constexpr uint8_t  PAGE2_SIZE         { 8 }; // NV2, nonvolatile
    static constexpr uint8_t  PAGE2_ADDR         { 0x20 };
    static constexpr uint8_t  PAGE3_SIZE         { 32 }; // SRAM, scratchpad
    static constexpr uint8_t  PAGE3_ADDR         { 0x40 };
    static constexpr uint8_t  PAGE4_SIZE         { 4 }; // SRAM, Temp & Status
    static constexpr uint8_t  PAGE4_ADDR         { 0x60 };
    static constexpr uint8_t  PAGE5_SIZE         { 4 }; // E2, Battery Cycle Counter & ID
    static constexpr uint8_t  PAGE5_ADDR         { 0x80 };
    static constexpr uint8_t  PAGE6_ADDR         { 0xA0 }; // LIMIT

    // pages simplified with 5x 32byte (there is enough RAM)
    static constexpr uint8_t  PAGE_SIZE          { 32 };
    static constexpr uint8_t  PAGE_COUNT         { 5 };

    static constexpr uint8_t  MEM_SIZE           { 3 * PAGE_SIZE };
    static constexpr uint8_t  SCRATCHPAD_SIZE    { PAGE_COUNT * PAGE_SIZE };

    static constexpr uint32_t DURATION_TEMP_ms   { 230 };
    static constexpr uint32_t DURATION_NVWR_ms   { 10 };
    uint32_t timer_temp = 0u;
    uint32_t timer_nvwr = 0u;
    bool     request_temp = false;

    uint8_t  memory[MEM_SIZE];
    uint8_t  scratchpad[SCRATCHPAD_SIZE];

    void    clearScratchpad(void);

public:

    static constexpr uint8_t family_code        { 0x53 }; // TODO: 1B seems to be right (for ds2436)

    DS2434(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    void    clearMemory(void);

    bool    writeMemory(const uint8_t* source, uint16_t length, uint16_t position = 0);
    bool    readMemory(uint8_t* destination, uint16_t length, uint16_t position = 0) const;

    void     setTemperature(int8_t temp_degC); // can vary from -40 to 127 degC
    bool     getTemperatureRequest(void) const;

    void     lockNV1();
    void     unlockNV1();

    void     setBatteryCounter(uint16_t value);

    void     setID(uint16_t value);
};

#endif //ONEWIREHUB_DS2434_H
