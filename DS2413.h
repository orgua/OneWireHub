// 0x3A  Dual channel addressable switch
// Work - 100%
class DS2413 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS2413(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
    
    bool AState;  // sensed.A
    bool ALatch;  // PIO.A
    bool BState;  // sensed.B
    bool BLatch;  // PIO.B
};
