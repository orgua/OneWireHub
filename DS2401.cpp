#include "OneWireHub.h"
#include "DS2401.h"

//#define DEBUG_DS2401

DS2401::DS2401(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

bool DS2401::duty(OneWireHub *hub)
{
    uint8_t done = hub->recv();

    switch (done)
    {
        default:
#ifdef DEBUG_hint
            Serial.print("DS2401=");
            Serial.println(done, HEX);
#endif
            break;
    }
}
