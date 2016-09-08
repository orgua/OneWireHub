#include "DS2438.h"

DS2438::DS2438(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (uint8_t i = 0; i < (PAGE_EMU_COUNT*8); ++i)
    {
        memory[i] = MemDS2438[i];
    }

    for (uint8_t page = 0; page <= PAGE_EMU_COUNT; ++page)
    {
        calcCRC(page);
    }

    setTemp(static_cast<int8_t>(20));
};

bool DS2438::duty(OneWireHub *hub)
{
    uint8_t page;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;

    switch (cmd)
    {
        // reordered for better timing
        case 0xBE: // Read Scratchpad
            page = hub->recv();
            if (hub->getError())  return false;
            if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT;

            hub->send(&memory[page * 8], 8);
            if (hub->getError())  return false;
            hub->send(crc[page]);
            break;

        case 0x4E: // Write Scratchpad
            page = hub->recv();
            if (hub->getError())  return false;

            if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT; // when page out of limits --> switch to garbage-page

            hub->recv(&memory[page * 8], 8);
            if (hub->getError())  return false;
            calcCRC(page);

            break;

        case 0x48: // copy scratchpad
            // do nother special, goto recall for now

        case 0xB8: // Recall Memory
            page = hub->recv();
            if (hub->getError())  return false;
            if (page >= PAGE_EMU_COUNT) page = PAGE_EMU_COUNT; // when page out of limits --> switch to garbage-page
            break;

        case 0x44: // Convert T
            //hub->sendBit(1); // 1 is passive, so ommit it ...
            break;

        case 0xB4: // Convert V
            //hub->sendBit(1); // 1 is passive, so ommit it ...
            break;

        default:
            hub->raiseSlaveError(cmd);
    };

    return !(hub->getError());
};

void DS2438::calcCRC(const uint8_t page)
{
    crc[page] = crc8(&memory[page * 8], 8);
}

void DS2438::setTemp(const float temp_degC)
{
    int16_t value = static_cast<int16_t>(temp_degC * 256.0);

    if (value > 125*256) value = 125*256;
    if (value < -55*256) value = -55*256;

    memory[1] = static_cast<uint8_t>(value&0xF8);
    memory[2] = uint8_t(value >> 8);

    calcCRC(0);
};

void DS2438::setTemp(const int8_t temp_degC) // can vary from -55 to 125deg
{
    int8_t value = temp_degC;
    if (value > 125) value = 125;
    if (value < -55) value = -55;

    memory[1] = 0;
    memory[2] = static_cast<uint8_t>(value);

    calcCRC(0);
};


void DS2438::setVolt(const uint16_t voltage_10mV)
{
    memory[3] = uint8_t(voltage_10mV & 0xFF);
    memory[4] = uint8_t((voltage_10mV >> 8) & static_cast<uint8_t>(0x03));
    calcCRC(0);
};

void DS2438::setCurr(const int16_t value) // signed 11 bit
{
    memory[5] = uint8_t(value & 0xFF);
    memory[6] = uint8_t((value >> 8) & static_cast<uint8_t>(0x03));
    if (value<0) memory[6] |= 0xFC; // all upper bits (7:2) are the signum
    calcCRC(0);
};