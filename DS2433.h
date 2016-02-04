// 0x23  4Kb 1-Wire EEPROM
class DS2433 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t memory[512];
};
