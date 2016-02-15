#include "OneWireHub.h"
#include "DS18B20.h"

//#define DEBUG_DS18B20

//=================== DS18S20 ==========================================
DS18B20::DS18B20(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
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
        case 0x44: // CONVERT T
            hub->sendBit(1);
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : CONVERT T");
#endif
            break;

        case 0x4E: // WRITE SCRATCHPAD
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : WRITE SCRATCHPAD");
#endif
            break;

        case 0xBE: // READ SCRATCHPAD
            hub->sendData(this->scratchpad, 9);
            if (hub->errno != ONEWIRE_NO_ERROR) return FALSE;
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : READ SCRATCHPAD");
#endif
            break;

        case 0x48: // COPY SCRATCHPAD
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : COPY SCRATCHPAD");
#endif
            break;

        case 0xB8: // RECALL E2
            hub->sendBit(1);
#ifdef DEBUG_DS18B20
            Serial.println("DS18B20 : RECALL E2");
#endif
            break;

        case 0xB4: // READ POWERSUPPLY
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


void DS18B20::settemp(float temperature_degC)
{
    word ret = 0;
    bool Neg = temperature_degC < 0;
    temperature_degC = abs(temperature_degC);
    ret = round(floor(temperature_degC)) << 4;

    if (Neg)
    {
        ret = ret | 0x8000;
    }

    ret = ret | uint8_t(16 * ((temperature_degC - (int) temperature_degC) * 100) / 100);

    this->scratchpad[0] = uint8_t(ret);
    this->scratchpad[1] = uint8_t(ret >> 8);
    updateCRC();
}

void settemp(int16_t temperature_degC)
{
    // TODO: implement
}