
class DS2450 : public OneWireItem
{
private:
    uint8_t memory[0x1F]; // TODO: make readable 3*8 or similar

    bool duty(OneWireHub *hub);
    static const uint8_t familycode = 0x20;

public:
    DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    bool setPotentiometer(const uint16_t p1, const uint16_t p2, const uint16_t p3, const uint16_t p4);
    bool setPotentiometer(const uint8_t number, const uint16_t value);

    //uint8_t Data[13];
    //bool updateCRC();
    //DS2450_memory memory;
};
