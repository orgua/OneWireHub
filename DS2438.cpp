#include "OneWireHub.h"
#include "DS2438.h"

DS2438::DS2438(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{

    for (int i = 0; i < sizeof(memory); ++i)
        memory[i] = MemDS2438[i]; // 0x00;

    setTemp(static_cast<uint8_t>(80));
/*
  // Flags
  memory[0] = DS2438_IAD | DS2438_CA | DS2438_EE | DS2438_AD;

  // Temp
  memory[1] = 0x10;
  memory[2] = 0x19;

  // Volt
  memory[3] = 0xF4;
  memory[4] = 0x01;

  // Cur
  memory[5] = 0x40;
  memory[6] = 0x00;
*/
}

bool DS2438::duty(OneWireHub *hub)
{
    uint8_t done = hub->recv();
    uint8_t page;
    uint8_t b; // TODO: unused
    uint8_t crc;

    switch (done)
    {
        // Convert T
        case 0x44:
            //hub->sendBit(1);
            if (dbg_sensor) Serial.println("DS2438 : Convert T");
            break;

            // Write Scratchpad
        case 0x4E:
            // page
            page = hub->recv();
            hub->recvData(&memory[page * 8], 8);

            if (dbg_sensor)
            {
                Serial.print("DS2438 : Write Scratchpad - Page:");
                Serial.println(page, HEX);
            }
            break;

            // Convert V
        case 0xB4:
            //hub->sendBit(1);
            if (dbg_sensor) Serial.println("DS2438 : Convert V");
            break;

            // Recall Memory
        case 0xB8:
            // page
            page = hub->recv();

            if (dbg_sensor)
            {
                Serial.print("DS2438 : Recall Memory - Page:");
                Serial.println(page, HEX);
            }
            break;

            // Read Scratchpad
        case 0xBE:
            // page
            page = hub->recv();

            //offset = page*8;

            // crc
            crc = crc8(&memory[page * 8], 8);

            hub->sendData(&memory[page * 8], 8);
            hub->send(crc);

            if (dbg_sensor)
            {
                Serial.print("DS2438 : Read Scratchpad - Page:");
                Serial.println(page, HEX);
            }
            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2438=");
                Serial.println(done, HEX);
            }
            break;
    }
    return TRUE;
}

void DS2438::setTemp(const float temp_degC)
{
    int16_t value = static_cast<int16_t>(temp_degC * 256.0);

    if (temp_degC < 0)
        value = -value;

    memory[1] = uint8_t(value);
    memory[2] = uint8_t(value >> 8);

    if (temp_degC < 0)
        memory[2] = memory[2] | static_cast<uint8_t>(0x80);

}

void DS2438::setTemp(const uint8_t temp_degC)
{
    memory[1] = 0;
    memory[2] = temp_degC;
    if (temp_degC < 0)
        memory[2] = memory[2] | static_cast<uint8_t>(0x80);
}


void DS2438::setVolt(const uint16_t voltage_10mV) // TODO: is this more than 10 bit? maybe use a mask
{
    memory[3] = uint8_t(voltage_10mV);
    memory[4] = uint8_t((voltage_10mV >> 8) & static_cast<uint8_t>(0x03));
}

void DS2438::setCurr(const uint16_t value) // TODO: signed 9 bit
{
    memory[5] = uint8_t(value);
    memory[6] = uint8_t((value >> 8) & static_cast<uint8_t>(0x03));
}