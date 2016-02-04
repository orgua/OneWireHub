// 0x29  8-Channel Addressable Switch @@@
// Not ready
#pragma pack(push, 1)
struct sDS2408
{
    byte cmd;
    byte adrL;
    byte adrH;
    byte D0;
    byte D1;
    byte D2;
    byte D3;
    byte D4;
    byte D5;
    byte D6;
    byte D7;
    uint16_t CRC;
};
#pragma pack(pop)

// Register Addresses
#define DS2408_PIO_LOGIC_REG       0x0088   // Current state
#define DS2408_PIO_OUTPUT_REG      0x0089   // Last write
#define DS2408_PIO_ACTIVITY_REG    0x008A   // State Change Activity
#define DS2408_SEARCH_MASK_REG     0x008B
#define DS2408_SEARCH_SELECT_REG   0x008C
#define DS2408_CONTROL_STATUS_REG  0x008D

class DS2408 : public OneWireItem
{
private:
    bool duty(OneWireHub *hub);

public:
    DS2408(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);

    byte memory[13];

    bool updateCRC();
};
