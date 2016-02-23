#include "OneWireHub.h"
#include "DS2408.h"

DS2408::DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    memory[0] = 0xF0;  // Cmd
    memory[1] = 0x88;  // AdrL
    memory[2] = 0x00;  // AdrH
    memory[3] = 0;     // D0
    memory[4] = 0;     // D1
    memory[5] = 0;     // D2
    memory[6] = 0;     // D3
    memory[7] = 0;     // D4
    memory[8] = 0;     // D5
    memory[9] = 0xFF;  // D6
    memory[10] = 0xFF; // D7
    memory[11] = 0;  // CRCL
    memory[12] = 0;  // CRCH
}

void DS2408::updateCRC()
{
    (reinterpret_cast<sDS2408 *>(memory))->CRC = ~crc16(memory, 11);
}

bool DS2408::duty(OneWireHub *hub)
{
    uint8_t data;  // TODO: unused for now
    uint16_t &crc = (reinterpret_cast<sDS2408 *>(memory))->CRC;
    crc = 0;
    uint8_t done = hub->recv();

    switch (done)
    {
        // Read PIO Registers
        case 0xF0:
//            memory[0] = done;        // Cmd
//            memory[1] = hub->recv(); // AdrL
//            memory[2] = hub->recv(); // AdrH
//            updateCRC();
//            data = hub->send(&memory[3], 10);

            crc = crc16(done, crc); // Test for now, above is the old code
            crc = crc16(hub->recv(), crc);
            crc = crc16(hub->recv(), crc);

            for (uint8_t count = 3; count < 11; ++count)
            {
                crc = hub->sendAndCRC16(memory[count], crc);
            }
            crc = ~crc; // most important step, easy to miss....
            hub->send(memory[11]);
            hub->send(memory[12]);

            if (dbg_sensor)
            {
                Serial.print("DS2408 : PIO Registers : ");
                Serial.print(memory[2], HEX);
                Serial.print(" ");
                Serial.print(memory[1], HEX);
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
