// 0x28  Digital Thermometer
// Work - 100%
class DS18B20 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS18B20(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);    
    
    byte scratchpad[9]; 
    bool updateCRC();
	
	void settemp(float temp);
};
