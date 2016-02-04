#include "OneWireHub.h"
#include "DS18B20.h"

//#define DEBUG_DS18B20

//=================== DS18S20 ==========================================
DS18B20::DS18B20(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    this->scratchpad[0] = 0xB4; // TLSB
    this->scratchpad[1] = 0x09; // TMSB
    this->scratchpad[2] = 0x4B; // THRE
    this->scratchpad[3] = 0x46; // TLRE
    this->scratchpad[4] = 0x1F; // Conf
    this->scratchpad[5] = 0xFF; // 0xFF
    this->scratchpad[6] = 0x00; // Rese
    this->scratchpad[7] = 0x10; // 0x10
    this->scratchpad[8] = 0x00; // CRC
    updateCRC();
}

bool DS18B20::updateCRC()
{
    this->scratchpad[8] = crc8(scratchpad, 8);
}

bool DS18B20::duty(OneWireHub *hub)
{
    uint8_t done = hub->recv();

    switch (done)
    {

        // CONVERT T
        case 0x44:
            hub->sendBit(1);

#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : CONVERT T");
#endif
            break;

            // WRITE SCRATCHPAD
        case 0x4E:
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : WRITE SCRATCHPAD");
#endif
            break;

            // READ SCRATCHPAD
        case 0xBE:
            hub->sendData(this->scratchpad, 9);
            if (hub->errno != ONEWIRE_NO_ERROR) return FALSE;

#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : READ SCRATCHPAD");
#endif
            break;

            // COPY SCRATCHPAD
        case 0x48:
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : READ SCRATCHPAD");
#endif
            break;

            // RECALL E2
        case 0xB8:
            hub->sendBit(1);

#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : RECALL E2");
#endif
            break;

            // READ POWERSUPPLY
        case 0xB4:
            hub->sendBit(1);

#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : READ POWERSUPPLY");
#endif
            break;

            //  write trim2               0x63
            //  copy trim2                0x64
            //  read trim2                0x68
            //  read trim1                0x93
            //  copy trim1                0x94
            //  write trim1               0x95

        default:
#ifdef DEBUG_hint
            Serial.print("DS18B20=");
            Serial.println(done, HEX);
#endif
            break;
    }

    return TRUE;
}

void DS18B20::settemp(float temp)
{
    word ret = 0;
    bool Neg = temp < 0;
    temp = abs(temp);
    ret = round(floor(temp)) << 4;

    if (Neg)
    {
        ret = ret | 0x8000;
    }

    ret = ret | byte(16 * ((temp - (int) temp) * 100) / 100);

    this->scratchpad[0] = byte(ret);
    this->scratchpad[1] = byte(ret >> 8);
    updateCRC();
}
