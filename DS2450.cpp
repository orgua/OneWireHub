#include "OneWireHub.h"
#include "DS2450.h"

#define DEBUG_DS2450

DS2450::DS2450(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
/*  ((sDS2450*)(this->Data))->cmd = 0;
  ((sDS2450*)(this->Data))->adr1 = 0;
  ((sDS2450*)(this->Data))->adr2 = 0;
  ((sDS2450*)(this->Data))->CA = 0x0102;
  ((sDS2450*)(this->Data))->CB = 0x0304;
  ((sDS2450*)(this->Data))->CC = 0x0506;
  ((sDS2450*)(this->Data))->CD = 0x0708;
  ((sDS2450*)(this->Data))->CRC = 0;

  memset(&this->memory, 0, sizeof(this->memory));
  uint16_t* p = (uint16_t*)&this->memory.control_status;
  *p++ = 0x8C08;
  *p++ = 0x8C08;
  *p++ = 0x8C08;
  *p++ = 0x8C08;
  *p++ = 0xFF00;
  *p++ = 0xFF00;
  *p++ = 0xFF00;
  *p++ = 0xFF00;
  this->memory.calibration[4] = 0x40;
*/

    memset(&this->memory, 0, sizeof(this->memory));
}

bool DS2450::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    uint16_t memory_address_start;
    uint8_t b;
    uint16_t crc;

    ow_crc16_reset();

    uint8_t done = hub->recv();
    switch (done)
    {
        // READ MEMORY
        case 0xAA:
            // Cmd
            ow_crc16_update(0xAA);

            // Adr1
            b = hub->recv();
            ((uint8_t *) &memory_address)[0] = b;
            ow_crc16_update(b);

            // Adr2
            b = hub->recv();
            ((uint8_t *) &memory_address)[1] = b;
            ow_crc16_update(b);

            memory_address_start = memory_address;

            for (int i = 0; i < 8; i++)
            {
                uint8_t b = this->memory[memory_address + i];
                hub->send(b);
            }

            crc = ow_crc16_get();
            hub->send(((uint8_t *) &crc)[0]);
            hub->send(((uint8_t *) &crc)[1]);

#ifdef DEBUG_DS2450
            Serial.print("DS2450 : READ MEMORY : ");
            Serial.println(memory_address_start, HEX);
#endif

            break;

        default:
#ifdef DEBUG_hint
            Serial.print("DS2450=");
            Serial.println(done, HEX);
#endif
            break;
    }

    return TRUE;
}

/*
bool DS2450::updateCRC(){
//  ((sDS2450*)(this->Data))->CRC = crc16( this->Data, 11 );
  
  ow_crc16_reset();      
  for (int i=0;i<11;i++) ow_crc16_update(this->Data[i]);
  ((sDS2450*)(this->Data))->CRC = ow_crc16_get();

  //Serial.print("CRC=");
  //Serial.println( ((sDS2408*)(this->memory))->CRC, HEX);
}
*/
