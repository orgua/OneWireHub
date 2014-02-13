// 0x2C  Single channel digital panemtiometer
// Work - 100%
class DS2890 : public OneWireItem{
    private:
    bool duty( OneWireHub * hub );
  public:
    DS2890(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
    
    byte regs;
    byte potion;
};
