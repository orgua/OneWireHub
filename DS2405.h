// 0x05  Single adress switch @@@
// Not ready
class DS2405 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);
};
