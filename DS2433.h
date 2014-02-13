// 0x23  4Kb 1-Wire EEPROM
class DS2433 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS2433(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
    
    byte memory[512];
 };
