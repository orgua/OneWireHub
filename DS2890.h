// 0x2C  Single channel digital panemtiometer
// Work - 100%

#ifndef ONEWIRE_DS2890_H
#define ONEWIRE_DS2890_H

class DS2890 : public OneWireItem
{
private:
    static constexpr bool    dbg_sensor  = 0; // give debug messages for this sensor
    static constexpr uint8_t family_code = 0x2C;

    uint8_t regs;
    uint8_t potion;

    bool duty(OneWireHub *hub);

public:
    DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);
};

#endif