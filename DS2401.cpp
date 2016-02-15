#include "OneWireHub.h"
#include "DS2401.h"

const bool dbg_DS2401 = 0; // give debug messages for this sensor

DS2401::DS2401(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
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
