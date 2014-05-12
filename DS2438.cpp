#include "OneWireHub.h"
#include "DS2438.h"

#define DEBUG_DS2438

static byte MemDS2438[64] =
    {
      0x09, 0x20, 0x14, 0xAC, 0x00, 0x40, 0x01, 0x00,
      0x5A, 0xC8, 0x05, 0x02, 0xFF, 0x08, 0x00, 0xFC, 
      0x00, 0x00, 0x00, 0x00, 0x6D, 0x83, 0x03, 0x02, 
      0xF2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
      0x26, 0xBF, 0x8E, 0x30, 0x01, 0x00, 0x00, 0x00, 
      0x2D, 0x07, 0xDB, 0x15, 0x00, 0x00, 0x00, 0x00, 
      0x28, 0x80, 0xDC, 0x1B, 0x03, 0x00, 0x00, 0x00, 
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

// 092014AC004001005AC80502FF0800FC000000006D830302F20000000000000026BF8E30010000002D07DB15000000002880DC1B030000000000000000000000 
// 0AE813EF010000003D980502000800FC0000000000000000F4000000000000003A78E90700000000000000000000000000000000000000000000000000000000
// 08B013C501000000D6920202000800FC0000000000000000F4000000000000003AFCF4070000000000000000000000000000000000000000F300000000000000
// 09D813D301020080FDE80702FF1000FC000000001BDAFF01F20000000000000000000000000000001207DA0D000000002828520E020000000000000000000000
// 080014C501000000BC850702000800FC0000000000000000000000000000000026BCBF30010000002C07DB1500000000284C9E1B030000000000000073010000

DS2438::DS2438(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7): OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7){

  for (int i=0;i<sizeof(this->memory);i++)
    this->memory[i] = MemDS2438[i]; // 0x00;

  SetTemp(80);
/*
  // Flags  
  this->memory[0] = DS2438_IAD | DS2438_CA | DS2438_EE | DS2438_AD;
  
  // Temp
  this->memory[1] = 0x10;
  this->memory[2] = 0x19;

  // Volt
  this->memory[3] = 0xF4;
  this->memory[4] = 0x01; 

  // Cur
  this->memory[5] = 0x40;
  this->memory[6] = 0x00; 
*/  
}

bool DS2438::duty(OneWireHub * hub)
{
  uint8_t done = hub->recv();  
  uint8_t page;
  uint8_t b;
  uint8_t crc;
 
  
  switch (done) {
    // Convert T
    case 0x44:
      //hub->sendBit(1);

      #ifdef DEBUG_DS2438
        Serial.println("DS2438 : Convert T");  
      #endif  
      break;
    
    // Write Scratchpad
    case 0x4E:
      // page
      page = hub->recv();
      
      hub->recvData(&this->memory[page*8], 8);
    
      #ifdef DEBUG_DS2438
        Serial.print("DS2438 : Write Scratchpad - Page:");  
        Serial.println(page, HEX);
      #endif  
      break;

    // Convert V
    case 0xB4:
      //hub->sendBit(1);

      #ifdef DEBUG_DS2438
        Serial.println("DS2438 : Convert V");  
      #endif  
      break;
    
    // Recall Memory
    case 0xB8:
      // page
      page = hub->recv();
   
      #ifdef DEBUG_DS2438
        Serial.print("DS2438 : Recall Memory - Page:");  
        Serial.println(page, HEX);
      #endif 
      break;

    // Read Scratchpad
    case 0xBE:
      // page
      page = hub->recv();

      //offset = page*8;
      
      // crc
      crc = crc8(&this->memory[page*8], 8);
      
      hub->sendData(&this->memory[page*8], 8);
      hub->send(crc);
      
      #ifdef DEBUG_DS2438
        Serial.print("DS2438 : Read Scratchpad - Page:");  
        Serial.println(page, HEX);       
      #endif  
      break;
      
    default:
      #ifdef DEBUG_hint
        Serial.print("DS2438=");
        Serial.println(done, HEX);
      #endif  
      break;    
  }
}

void DS2438::SetTemp(float temp)
{
  memory[1] = byte(256*((temp - (int)temp) * 100)/100);
  memory[2] = round(abs(floor(temp)));
  
  if (temp < 0){ memory[2] = memory[2] | 0x80; }
}

void DS2438::SetVolt(word val)
{
  memory[3] = byte(val);
  memory[4] = byte(val >> 8);
}

void DS2438::SetCurr(word val)
{
  memory[5] = byte(val);
  memory[6] = byte(val >> 8);
}