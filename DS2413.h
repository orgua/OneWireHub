// 0x3A  Dual channel addressable switch
// Work - 100%

#ifndef ONEWIRE_DS2413_H
#define ONEWIRE_DS2413_H

class DS2413 : public OneWireItem
{
private:
    static constexpr bool    dbg_sensor  = 0; // give debug messages for this sensor
    static constexpr uint8_t family_code = 0x3A;

    bool duty(OneWireHub *hub);

public:
    bool AState;  // sensed.A
    bool ALatch;  // PIO.A
    bool BState;  // sensed.B
    bool BLatch;  // PIO.B

    DS2413(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    virtual void ReadState();

    virtual void ChangePIO();
};

#endif