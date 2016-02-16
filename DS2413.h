// 0x3A  Dual channel addressable switch
// Work - 100%
class DS2413 : public OneWireItem
{
private:
    bool AState;  // sensed.A
    bool ALatch;  // PIO.A
    bool BState;  // sensed.B
    bool BLatch;  // PIO.B

    bool duty(OneWireHub *hub);

public:
    DS2413(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    virtual void ReadState();

    virtual void ChangePIO();
};
