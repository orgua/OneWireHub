// 0x01  Serial Number
// Work - 100%

class DS2401 : public OneWireItem
{
private:
    constexpr static uint8_t family_code = 0x01;
    bool duty(OneWireHub *hub);

public:
    DS2401(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(family_code, ID2, ID3, ID4, ID5, ID6, ID7)
    {
    };
    /*
    DS2401(uint8_t SER0, uint8_t SER1, uint8_t SER2, uint8_t SER3, uint8_t SER4, uint8_t SER5) : DS2401(family_code,SER0,SER1,SER2,SER3,SER4,SER5)
    {
    };
     */
};
