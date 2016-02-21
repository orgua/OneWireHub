// 0x20 4 channel A/D
//

#ifndef ONEWIRE_DS2450_H
#define ONEWIRE_DS2450_H

class DS2450 : public OneWireItem
{
private:
    static constexpr bool    dbg_sensor  = 0; // give debug messages for this sensor

    uint8_t memory[0x1F]; // TODO: make readable 3*8 or similar

    bool duty(OneWireHub *hub);

public:
    static constexpr uint8_t family_code = 0x20;

    DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool setPotentiometer(const uint16_t p1, const uint16_t p2, const uint16_t p3, const uint16_t p4);
    bool setPotentiometer(const uint8_t number, const uint16_t value);

    //uint8_t Data[13];
    //bool updateCRC();
    //DS2450_memory memory;
};

#endif