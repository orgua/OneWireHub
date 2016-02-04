// 0x26  Smart Battery Monitor

// Register Addresses
#define DS2438_IAD  0x01
#define DS2438_CA   0x02
#define DS2438_EE   0x04
#define DS2438_AD   0x08
#define DS2438_TB   0x10
#define DS2438_NVB  0x20
#define DS2438_ADB  0x40

#pragma pack(push, 1)
struct DS2438_page0
{
    byte flags;
    int16_t temp;
    int16_t volt;
    int16_t curr;
    byte threshold;
};
#pragma pack(pop)

class DS2438 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2438(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);

    void SetTemp(float temp);

    void SetVolt(word val);

    void SetCurr(word val);

    byte memory[64];
};
