#include "DS2408.h"

DS2408::DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    memory.field.cmd = 0xF0;  // Cmd
    memory.bytes[1] = 0x88;  // AdrL
    memory.bytes[2] = 0x00;  // AdrH
    memory.bytes[3] = 0;     // D0
    memory.bytes[4] = 0;     // D1
    memory.bytes[5] = 0;     // D2
    memory.bytes[6] = 0;     // D3
    memory.bytes[7] = 0;     // D4
    memory.bytes[8] = 0;     // D5
    memory.bytes[9] = 0xFF;  // D6
    memory.bytes[10] = 0xFF; // D7
    memory.bytes[11] = 0;  // CRCL
    memory.bytes[12] = 0;  // CRCH
}

void DS2408::updateCRC()
{
    //(reinterpret_cast<sDS2408 *>(memory))->CRC = ~crc16(memory, 11);
}

bool DS2408::duty(OneWireHub *hub)
{
    memory.field.crc = 0;
    uint8_t done = hub->recv();

    switch (done)
    {
        // Read PIO Registers
        case 0xF0:
            memory.field.crc = crc16(done, memory.field.crc); // Test for now, above is the old code
            memory.field.crc = crc16(hub->recv(), memory.field.crc);
            memory.field.crc = crc16(hub->recv(), memory.field.crc);

            for (uint8_t count = 3; count < 11; ++count)
            {
                memory.field.crc = hub->sendAndCRC16(memory.bytes[count], memory.field.crc);
            }
            memory.field.crc = ~memory.field.crc; // most important step, easy to miss....
            hub->send(memory.bytes[11]);
            hub->send(memory.bytes[12]);

            if (dbg_sensor)
            {
                Serial.print("DS2408 : PIO Registers : ");
                Serial.print(memory.bytes[2], HEX);
                Serial.print(" ");
                Serial.print(memory.bytes[1], HEX);
                Serial.println();
            }

            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2408=");
                Serial.println(done, HEX);
            }
            return false;
    }

    return true;
}
