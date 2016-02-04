// 0x05  Single adress switch @@@
// Not ready
class DS2405 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2405(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
};
