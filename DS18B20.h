// 0x28  Digital Thermometer
// Work - 100%
class DS18B20 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS18B20(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t scratchpad[9];

    bool updateCRC();

    void settemp(const float   temperature_degC);
    void settemp(const int16_t temperature_degC);
};
