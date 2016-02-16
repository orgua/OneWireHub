#include "OneWireHub.h"
#include "DS2408.h"

const bool dbg_DS2408 = 0; // give debug messages for this sensor

//=================== DS2408 ==========================================
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

bool DS2408::updateCRC()
{
    ow_crc16_reset();
    for (int i = 0; i < 11; ++i) ow_crc16_update(memory[i]);
    (reinterpret_cast<sDS2408 *>(memory))->CRC = ow_crc16_get();
}

bool DS2408::duty(OneWireHub *hub)
{
    uint8_t addrL; // TODO: unused for now
    uint8_t addrH; // TODO: unused for now
    uint8_t data;  // TODO: unused for now

    uint8_t done = hub->recv();

    switch (done)
    {
        // Read PIO Registers
        case 0xF0:
            // Cmd
            memory[0] = done;

            // AdrL
            memory[1] = hub->recv();

            // AdrH
            memory[2] = hub->recv();

            // UpdateCrc
            updateCRC();

            // Data
            data = hub->sendData(memory + 3, 10);

            if (dbg_DS2408)
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
            return FALSE;
    }

    return TRUE;
}
