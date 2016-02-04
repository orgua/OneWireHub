// 0x2C  Single channel digital panemtiometer
// Work - 100%
class DS2890 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t regs;
    uint8_t potion;
};
