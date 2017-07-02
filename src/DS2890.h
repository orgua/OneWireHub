// Single channel digital potentiometer
// Works, is prepared for four channels
// native bus-features: Overdrive capable

#ifndef ONEWIRE_DS2890_H
#define ONEWIRE_DS2890_H

#include "OneWireItem.h"

class DS2890 : public OneWireItem
{
private:

    static constexpr uint8_t POTI_SIZE               { 4 }; // number of potis emulated
    static constexpr uint8_t POTI_MASK               { 0b00000011 };

    static constexpr uint8_t REG_MASK_POTI_CHAR      { 0b00000001 }; // 0: log, 1: linear
    static constexpr uint8_t REG_MASK_WIPER_SET      { 0b00000010 }; // 0: non, 1: volatile
    static constexpr uint8_t REG_MASK_POTI_NUMB      { 0b00001100 }; // 0..4 potis
    static constexpr uint8_t REG_MASK_WIPER_POS      { 0b00110000 }; // 32, 64, 128, 256 positions
    static constexpr uint8_t REG_MASK_POTI_RESI      { 0b11000000 }; // 5k, 10k, 50k, 100k Ohm Resistence

    static constexpr uint8_t RELEASE_CODE            { 0x96 };

    uint8_t register_feat;
    uint8_t register_ctrl;
    uint8_t register_poti[POTI_SIZE];

public:

    static constexpr uint8_t family_code            { 0x2C };

    DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void    duty(OneWireHub * hub) final;

    void    setPotentiometer(uint8_t channel, uint8_t value)
    {
        register_poti[channel&POTI_MASK] = value;
    }

    uint8_t getPotentiometer(uint8_t channel) const
    {
        return register_poti[channel&POTI_MASK];
    }

    uint8_t getRegCtrl(void) const
    {
        return register_ctrl;
    }

    uint8_t getRegFeat(void) const
    {
        return register_feat;
    }
};

#endif
