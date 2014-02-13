// 0x1D  4kb 1-Wire RAM with Counter
class DS2423 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS2423(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
};
