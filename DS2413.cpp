#include "OneWireHub.h"
#include "DS2413.h"

//#define DEBUG_DS2413

DS2413::DS2413(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    this->AState = true; //false;
    this->ALatch = false;
    this->BState = false;
    this->BLatch = false;
}

bool DS2413::duty(OneWireHub *hub)
{
    uint8_t done = hub->recv();
    uint8_t data = 0;

    switch (done)
    {
        // PIO ACCESS WRITE
        case 0x5A:
            data = ~hub->recv();

            // Write inverse
            hub->send(data);

            //@@@ - ToDo event
            this->ALatch = data & 0x01;
            this->BLatch = data & 0x02;

#ifdef DEBUG_DS2413
        Serial.print("DS2413 : PIO WRITE  : 5A = ");
        Serial.println(data, HEX);
#endif

            this->ChangePIO();

            break;

            // PIO ACCESS READ
        case 0xF5:
            this->ReadState();

            data = 0;
            if (this->AState) data = data | 0x01;
            if (!this->ALatch) data = data | 0x02;
            if (this->BState) data = data | 0x04;
            if (!this->BLatch) data = data | 0x08;

            data = data | (~data << 4);
            hub->send(data);

#ifdef DEBUG_DS2413
        Serial.print("DS2413 : PIO ACCESS READ : F5 = ");
        Serial.println(data, HEX);
#endif

            break;

        default:
#ifdef DEBUG_hint
            Serial.print("DS2413=");
            Serial.println(done, HEX);
#endif
            break;
    }

    return TRUE;
}

void DS2413::ReadState()
{
}

void DS2413::ChangePIO()
{
}