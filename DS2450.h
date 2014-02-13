// 0x20  4 channel A/D
/*
#pragma pack(push, 1)
struct sDS2450
{
  byte cmd;
  byte adr1;
  byte adr2;
  int16_t CA;
  int16_t CB; 
  int16_t CC; 
  int16_t CD; 
  uint16_t CRC;
};
#pragma pack(pop)

struct DS2450_memory {
        uint16_t conversion_readout[4]; // page 0
        struct {
                unsigned int rc:4; // number of bits, 0000 = 16
                unsigned int : 2;
                unsigned int oc:1; // output value
                unsigned int oe:1; // output enable
                unsigned int ir:1; // 0 - half max voltage, 1 - full max voltage
                unsigned int : 1;
                unsigned int ael:1; // alarm enable low
                unsigned int aeh:1; // alarm enable high
                unsigned int afl:1; // alarm flag low
                unsigned int afh:1; // alarm flag high
                unsigned int : 1;
                unsigned int por:1; // just powered on
        } control_status[4]; // page 1
        struct {
                uint8_t low;
                uint8_t high;
        } alarm_settings[4]; // page 2
        uint8_t calibration[8]; // page3
};
*/

class DS2450 : public OneWireItem{
  private:
    bool duty( OneWireHub * hub );
  public:
    DS2450(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);
    
    byte memory[0x1F];
    
    //byte Data[13];
    //bool updateCRC();
    //DS2450_memory memory;
};
