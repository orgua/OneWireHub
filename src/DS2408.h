// 0x29  8-Channel Addressable Switch @@@
// Not ready

#ifndef ONEWIRE_DS2408_H
#define ONEWIRE_DS2408_H

#include "OneWireItem.h"

#pragma pack(push, 1)
struct sDS2408 // TODO: could be overlayed with struct
{
    uint8_t cmd;
    uint8_t adrL;
    uint8_t adrH;
    uint8_t D0;
    uint8_t D1;
    uint8_t D2;
    uint8_t D3;
    uint8_t D4;
    uint8_t D5;
    uint8_t D6;
    uint8_t D7;
    uint16_t CRC;
};
#pragma pack(pop)


class DS2408 : public OneWireItem
{
private:
    static constexpr bool    dbg_sensor  = 0; // give debug messages for this sensor

    // Register Addresses
    static constexpr uint8_t  DS2408_RD_BELOW_UNDEFINED  = 0x87;  // undefined data
    static constexpr uint8_t  DS2408_PIO_LOGIC_REG       = 0x88;  // Current state
    static constexpr uint8_t  DS2408_PIO_OUTPUT_REG      = 0x89;  // Last write, latch state register
    static constexpr uint8_t  DS2408_PIO_ACTIVITY_REG    = 0x8A;  // State Change Activity
    static constexpr uint8_t  DS2408_SEARCH_MASK_REG     = 0x8B;  // RW
    static constexpr uint8_t  DS2408_SEARCH_SELECT_REG   = 0x8C;  // RW
    static constexpr uint8_t  DS2408_CONTROL_STATUS_REG  = 0x8D;  // RW Control Register
    static constexpr uint8_t  DS2408_RD_ABOVE_ALWAYS_FF  = 0x8E;  // these bytes give always 0xFF

    uint8_t memory[13];

    void updateCRC(void);

public:
    static constexpr uint8_t family_code = 0x29;

    DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);


};

#endif