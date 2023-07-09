// 256bit 1-Wire EEPROM & 64bit OTP
// works

#ifndef ONEWIRE_DS2430_H
#define ONEWIRE_DS2430_H

#include "OneWireItem.h"

class DS2430 : public OneWireItem
{
private:
    static constexpr uint8_t MEM_SIZE{32 + 8};

    static constexpr uint8_t SCRATCHPAD_SIZE{32 + 8};

    static constexpr uint8_t SCRATCHPAD1_MASK{0b00011111};
    static constexpr uint8_t SCRATCHPAD1_SIZE{32};

    static constexpr uint8_t SCRATCHPAD2_ADDR{32};
    static constexpr uint8_t SCRATCHPAD2_SIZE{8};
    static constexpr uint8_t SCRATCHPAD2_MASK{0b00000111};

    uint8_t memory[MEM_SIZE];

    uint8_t scratchpad[SCRATCHPAD_SIZE];

    uint8_t status_register;

    void clearScratchpad(void);

public:
    static constexpr uint8_t family_code{0x14};

    DS2430(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6,
           uint8_t ID7);

    void duty(OneWireHub *hub) final;

    void clearMemory(void);

    bool writeMemory(const uint8_t *source, uint8_t length, uint8_t position = 0);
    bool readMemory(uint8_t *destination, uint16_t length, uint16_t position = 0) const;

    bool syncScratchpad(void);
    // this FN copies content of memory to scratchpad
    // needed because programming interface only allows access to memory
};

#endif
