// 0x05  Single adress switch @@@
// Not ready

#ifndef ONEWIRE_DS2405_H
#define ONEWIRE_DS2405_H

class DS2405 : public OneWireItem
{
private:
    static constexpr bool    dbg_sensor  = 0; // give debug messages for this sensor
    static constexpr uint8_t family_code = 0x05;

    bool duty(OneWireHub *hub);

public:
    DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);
};

#endif