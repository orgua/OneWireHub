#include "OneWireHub.h"
#include "DS2405.h"

#define DEBUG_DS2405

//=================== DS2405 ==========================================
DS2405::DS2405(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

bool DS2405::duty(OneWireHub *hub)
{
    uint8_t done = hub->recv();

    switch (done)
    {
        // Active-Only Search ROM
        // EC - @@@
        default:
#ifdef DEBUG_hint
            Serial.print("DS2405=");
            Serial.println(done, HEX); // 00 - Set ON
#endif
            break;
    }
}

