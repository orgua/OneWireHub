// 0x01  Serial Number
// Work - 100%
class DS2401 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS2401(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);    
};
