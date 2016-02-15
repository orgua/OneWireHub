#include "OneWireHub.h"
#include "DS18B20.h"

const bool dbg_DS18B20 = 0; // give debug messages for this sensor

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
            if (dbg_DS18B20) Serial.println("DS18B20 : CONVERT T");
            break;

        case 0x4E: // WRITE SCRATCHPAD
            if (dbg_DS18B20) Serial.println("DS18B20 : WRITE SCRATCHPAD");
            break;

        case 0xBE: // READ SCRATCHPAD
            hub->sendData(this->scratchpad, 9);
            if (hub->errno != ONEWIRE_NO_ERROR) return FALSE;
            if (dbg_DS18B20) Serial.println("DS18B20 : READ SCRATCHPAD");
            break;

        case 0x48: // COPY SCRATCHPAD
            if (dbg_DS18B20) Serial.println("DS18B20 : COPY SCRATCHPAD");
            break;

        case 0xB8: // RECALL E2
            hub->sendBit(1);
            if (dbg_DS18B20) Serial.println("DS18B20 : RECALL E2");
            break;

        case 0xB4: // READ POWERSUPPLY
            hub->sendBit(1);
            if (dbg_DS18B20) Serial.println("DS18B20 : READ POWERSUPPLY");
            break;

            //  write trim2               0x63
            //  copy trim2                0x64
            //  read trim2                0x68
            //  read trim1                0x93
            //  copy trim1                0x94
            //  write trim1               0x95

        default:
            if (dbg_HINT)
            {
                Serial.print("DS18B20=");
                Serial.println(done, HEX);
            }
            break;
    }

    return TRUE;
}


void DS18B20::settemp(const float temperature_degC)
{
    bool neg = 0;
    int16_t value = static_cast<int16_t>(temperature_degC * 16.0);

    if (value > 0)
    {
        value &= 0x0FFF;
    }
    else
    {
        value = -value;
        value |= 0xF000;
    }

    this->scratchpad[0] = uint8_t(value);
    this->scratchpad[1] = uint8_t(value >> 8);
    updateCRC();
}

void DS18B20::settemp(const int16_t temperature_degC)
{
    bool neg = 0;
    int16_t value = temperature_degC * 16;

    if (value > 0)
    {
        value &= 0x0FFF;
    }
    else
    {
        value = -value;
        value |= 0xF000;
    }

    this->scratchpad[0] = uint8_t(value);
    this->scratchpad[1] = uint8_t(value >> 8);
    updateCRC();
}