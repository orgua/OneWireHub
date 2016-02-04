#include "OneWireHub.h"
#include "DS2408.h"

#define DEBUG_DS2408

//=================== DS2408 ==========================================
DS2408::DS2408(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    this->memory[0] = 0xF0;  // Cmd
    this->memory[1] = 0x88;  // AdrL
    this->memory[2] = 0x00;  // AdrH
    this->memory[3] = 0;     // D0
    this->memory[4] = 0;     // D1
    this->memory[5] = 0;     // D2
    this->memory[6] = 0;     // D3
    this->memory[7] = 0;     // D4
    this->memory[8] = 0;     // D5
    this->memory[9] = 0xFF;  // D6
    this->memory[10] = 0xFF; // D7
    this->memory[11] = 0;  // CRCL
    this->memory[12] = 0;  // CRCH
}

bool DS2408::updateCRC()
{
//  ((sDS2408*)(this->memory))->CRC = crc16( this->memory, 11 );

    ow_crc16_reset();
    for (int i = 0; i < 11; i++) ow_crc16_update(this->memory[i]);
    ((sDS2408 *) (this->memory))->CRC = ow_crc16_get();

    //Serial.print("CRC=");
    //Serial.println( ((sDS2408*)(this->memory))->CRC, HEX);
}

bool DS2408::duty(OneWireHub *hub)
{
    byte addrL;
    byte addrH;
    uint8_t data;

    uint8_t done = hub->recv();

    switch (done)
    {
        // Read PIO Registers
        case 0xF0:
            // Cmd
            this->memory[0] = done;

            // AdrL
            this->memory[1] = hub->recv();

            // AdrH
            this->memory[2] = hub->recv();

            // UpdateCrc
            this->updateCRC();

            // Data
            data = hub->sendData(this->memory + 3, 10);

#ifdef DEBUG_DS2408
            Serial.print("DS2408 : PIO Registers : ");
            Serial.print(this->memory[2], HEX);
            Serial.print(" ");
            Serial.print(this->memory[1], HEX);
            Serial.println();
#endif

            break;

        default:
#ifdef DEBUG_hint
            Serial.print("DS2408=");
            Serial.println(done, HEX);
#endif
            return FALSE;
    }

    return TRUE;
}
