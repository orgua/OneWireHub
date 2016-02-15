#include "OneWireHub.h"
#include "DS2405.h"

const bool dbg_DS2405 = 0; // give debug messages for this sensor

//=================== DS2405 ==========================================
DS2405::DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
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
            if (dbg_HINT)
            {
                Serial.print("DS2405=");
                Serial.println(done, HEX); // 00 - Set ON
            }
            break;
    }
}

