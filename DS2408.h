// 0x29  8-Channel Addressable Switch @@@
// Not ready
#pragma pack(push, 1)
struct sDS2408 // TODO: could be overlayed with struct
{
    uint8_t cmd;
    uint8_t adrL;
    uint8_t adrH;
    uint8_t D0;
    uint8_t D1;
    uint8_t D2;
    uint8_t D3;
    uint8_t D4;
    uint8_t D5;
    uint8_t D6;
    uint8_t D7;
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

    uint8_t memory[13];

    bool duty(OneWireHub *hub);

public:
    DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void updateCRC();
};
