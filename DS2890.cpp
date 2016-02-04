#include "OneWireHub.h"
#include "DS2890.h"

//#define DEBUG_DS2890

DS2890::DS2890(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    this->regs = 0;
    this->potion = 0;
}

bool DS2890::duty(OneWireHub *hub)
{
    uint16_t data;

    uint8_t done = hub->recv();
    switch (done)
    {
        // WRITE POSITION
        case 0x0F:
            // get
            this->potion = hub->recv();

            // send
            hub->send(this->potion);

#ifdef DEBUG_DS2890
        Serial.print("DS2890 : WRITE POSITION: ");
        Serial.println(this->potion, HEX);
#endif

            break;

            // WRITE CONTROL REGISTER
        case 0x55:
            // data
            this->regs = hub->recv();

            // send dara
            hub->send(this->regs);

#ifdef DEBUG_DS2890
        Serial.print("DS2890 : WRITE CONTROL REGISTER: ");
        Serial.println(this->regs, HEX);
#endif

            break;

            // READ CONTROL REGISTER
        case 0xAA:
            // regs
            hub->send(this->regs);

            // send
            hub->send(this->potion);

#ifdef DEBUG_DS2890
        Serial.print("DS2890 : READ CONTROL REGISTER: ");
        Serial.print(this->regs, HEX); 
        Serial.print("-");
        Serial.println(this->potion, HEX);
#endif

            break;

            // READ POSITION
        case 0xF0:
            // regs
            hub->send(this->regs);

            // send
            hub->send(this->potion);

#ifdef DEBUG_DS2890
        Serial.print("DS2890 : READ POSITION: ");
        Serial.print(this->regs, HEX); 
        Serial.print("-");
        Serial.println(this->potion, HEX);
#endif

            break;

            // INCREMENT
        case 0xC3:
            if (this->potion < 0xFF) this->potion = this->potion + 1;

            // send
            hub->send(this->potion);

#ifdef DEBUG_DS2890
            Serial.print("DS2890 : INCREMENT");
#endif

            break;

            // DECREMENT
        case 0x99:
            if (this->potion > 0x00) this->potion = this->potion - 1;

            // send
            hub->send(this->potion);

#ifdef DEBUG_DS2890
            Serial.print("DS2890 : DECREMENT");
#endif

            break;

        default:
#ifdef DEBUG_hint
            Serial.print("DS2890=");
            Serial.println(done, HEX);
#endif
            break;
    }

    return TRUE;
}
