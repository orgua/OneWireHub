// 0x28  Digital Thermometer
// Work - 100%

#ifndef ONEWIRE_DS18B20_H
#define ONEWIRE_DS18B20_H

class DS18B20 : public OneWireItem
{
private:
    uint8_t scratchpad[9];

    bool duty(OneWireHub *hub);
    void setTempRaw(const int16_t value_raw);
    void updateCRC();

public:
    DS18B20(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void setTemp(const float   temperature_degC);
    void setTemp(const int16_t temperature_degC);
};

#endif