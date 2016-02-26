// 0x2C  Single channel digital potentiometer
// Work - 100%

#ifndef ONEWIRE_DS2890_H
#define ONEWIRE_DS2890_H

#include "OneWireItem.h"

class DS2890 : public OneWireItem
{
private:

    static constexpr uint8_t REGISTER_MASK_POTI_CHAR = 0x01;
    static constexpr uint8_t REGISTER_MASK_WIPER_SET = 0x02;
    static constexpr uint8_t REGISTER_MASK_POTI_NUMB = 0x0C;
    static constexpr uint8_t REGISTER_MASK_WIPER_POS = 0x30;
    static constexpr uint8_t REGISTER_MASK_POTI_RESI = 0xC0;

    uint8_t register_feat;
    uint8_t register_ctrl;
    uint8_t register_poti[4];

public:
    static constexpr uint8_t family_code = 0x2C;

    DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool duty(OneWireHub *hub);

    uint8_t readPoti(uint8_t number)
    {
        return register_poti[number&0x03];
    }
    uint8_t readCtrl(void)
    {
        return register_ctrl;
    }
    uint8_t readFeat(void)
    {
        return register_feat;
    }
};

#endif