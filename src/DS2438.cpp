#include "DS2438.h"

DS2438::DS2438(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (uint8_t i = 0; i < (PAGE_EMU_COUNT*8); ++i)
        memory[i] = MemDS2438[i]; // 0x00;

    setTemp(static_cast<uint8_t>(80));
};

bool DS2438::duty(OneWireHub *hub)
{
    static uint8_t page;
    static uint8_t crc;
    uint8_t page_new;

    uint8_t cmd = hub->recv();
    hub->extendTimeslot(); // test

    switch (cmd)
    {
        // reordered for better timing during debug
        case 0xBE: // Read Scratchpad
            page_new = hub->recv();

            if (page_new != page) // this case may produce errors --> we should recalc crc on the fly after changing something
            {
                page = page_new;
                if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT;
                crc = crc8(&memory[page * 8], 8);
            }
            hub->send(&memory[page * 8], 8);
            hub->send(crc);
            break;

        case 0x4E: // Write Scratchpad
            page = hub->recv();

            if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT;

            hub->recv(&memory[page * 8], 8);
            crc = crc8(&memory[page * 8], 8);

            break;

        case 0x48: // copy scratchpad
            // do nother special, goto recall for now

        case 0xB8: // Recall Memory
            page = hub->recv();

            if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT;

            crc = crc8(&memory[page * 8], 8);

            break;

        case 0x44: // Convert T
            //hub->sendBit(1); // 1 is passive, so ommit it ...
            break;

        case 0xB4: // Convert V
            //hub->sendBit(1); // 1 is passive, so ommit it ...
            break;

        default:
            hub->raiseSlaveError(cmd);
            break;
    };
    //Serial.print(cmd,HEX);
    return true;
};

void DS2438::setTemp(const float temp_degC)
{
    int16_t value = static_cast<int16_t>(temp_degC * 256.0);

    if (temp_degC < 0)
        value = -value;

    memory[1] = static_cast<uint8_t>(value&0xF8);
    memory[2] = uint8_t(value >> 8);

    if (temp_degC < 0)
        memory[2] = memory[2] | static_cast<uint8_t>(0x80);
};

void DS2438::setTemp(const uint8_t temp_degC)
{
    memory[1] = 0;
    memory[2] = temp_degC;
    if (temp_degC < 0) // TODO: how should this value fall below 0, unsigned
        memory[2] = memory[2] | static_cast<uint8_t>(0x80);
};


void DS2438::setVolt(const uint16_t voltage_10mV)
{
    memory[3] = uint8_t(voltage_10mV & 0xFF);
    memory[4] = uint8_t((voltage_10mV >> 8) & static_cast<uint8_t>(0x03));
};

void DS2438::setCurr(const int16_t value) // signed 11 bit
{
    memory[5] = uint8_t(value & 0xFF);
    memory[6] = uint8_t((value >> 8) & static_cast<uint8_t>(0x03));
    if (value<0) memory[6] |= 0xFC; // all upper bits (7:2) are the sign
};