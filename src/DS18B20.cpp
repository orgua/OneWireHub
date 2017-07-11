#include "DS18B20.h"


DS18B20::DS18B20(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    scratchpad[0] = 0xA0; // TLSB --> 10 degC as std
    scratchpad[1] = 0x00; // TMSB
    scratchpad[2] = 0x4B; // THRE --> Trigger register TH
    scratchpad[3] = 0x46; // TLRE --> TLow
    scratchpad[4] = 0x7F; // Conf
    // = 0 R1 R0 1 1 1 1 1 --> R=0 9bit .... R=3 12bit
    scratchpad[5] = 0xFF; // 0xFF
    scratchpad[6] = 0x00; // Reset
    scratchpad[7] = 0x10; // 0x10
    updateCRC(); // update scratchpad[8]

    ds18s20_mode = (ID1 == 0x10); // different tempRegister
}

void DS18B20::updateCRC()
{
    scratchpad[8] = crc8(scratchpad, 8);
}

void DS18B20::duty(OneWireHub * const hub)
{
    uint8_t cmd;
    if (hub->recv(&cmd,1)) return;

    switch (cmd)
    {
        case 0x4E: // WRITE SCRATCHPAD
            // write 3 byte of data to scratchpad[2:4], ds18s20 only first 2 bytes (TH, TL)
            hub->recv(&scratchpad[2], 3); // dont return here, so crc gets updated even if write not complete
            updateCRC();
            break;

        case 0xBE: // READ SCRATCHPAD
            hub->send(scratchpad, 9);
            break;

        case 0x48: // COPY SCRATCHPAD to EEPROM
            // todo: we could implement eprom here und below, copy scratchpad[2:4], ds18s20 only first 2 bytes (TH, TL)
            break; // send1 if parasite power is used, is passive

        case 0xB8: // RECALL E2 (3 byte EEPROM to Scratchpad[2:4])
            break;// signal that OP is done, 1s is passive ...

        case 0xB4: // READ POWER SUPPLY
            //hub->sendBit(0); // 1: say i am external powered, 0: uses parasite power, 1 is passive, so omit it ...
            break;

        case 0x44: // CONVERT T --> start a new measurement conversion
            // we have 94 ... 750ms time here (9-12bit conversion)
            break; // send 1s, is passive ...

        default:
            hub->raiseSlaveError(cmd);
    }
}


void DS18B20::setTemperature(const float value_degC)
{
    float value = value_degC;
    if (value > 125) value = 125;
    if (value < -55) value = -55;
    setTemperatureRaw(static_cast<int16_t>(value * 16.0f));
}

void DS18B20::setTemperature(const int8_t value_degC) // could be int8_t, [-55;+85] degC
{
    int8_t value = value_degC;
    if (value > 125) value = 125;
    if (value < -55) value = -55;
    setTemperatureRaw(value * static_cast<int8_t>(16));
}

void DS18B20::setTemperatureRaw(const int16_t value_raw)
{
    int16_t value = value_raw;

    if (ds18s20_mode)
    {
        value /= 8; // deg*16/8 = deg*2 ...
        if (value >= 0)
        {
            value &= 0x00FF; // upper byte is signum (0)
        }
        else
        {
            value |= 0xFF00; // upper byte is signum (1)
        }
    }
    else
    {
        // normal 18b20, uses always 12bit mode! also 9,10,11,12 bit possible bitPosition seems to stay the same
        if (value >= 0)
        {
            value &= 0x07FF; // upper 5 bits are signum
        }
        else
        {
            value |= 0xF800;
        }
    }

    scratchpad[0] = reinterpret_cast<uint8_t *>(&value)[0];
    scratchpad[1] = reinterpret_cast<uint8_t *>(&value)[1];
    // TODO: if alarms implemented - compare TH,TL with (value>>4 & 0xFF) (bit 11to4)
    // if out of bounds >=TH, <=TL trigger flag

    updateCRC();
}

int  DS18B20::getTemperature(void) const
{
    int16_t value = getTemperatureRaw();

    if (ds18s20_mode)
    {
        value /= 2;
    }
    else
    {
        value /= 16;
    }

    return value;
}

int16_t DS18B20::getTemperatureRaw() const
{
    return static_cast<int16_t>((scratchpad[1] << 8) | scratchpad[0]);
}
