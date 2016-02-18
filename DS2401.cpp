#include "OneWireHub.h"
#include "DS2401.h"

const bool dbg_DS2401 = 0; // give debug messages for this sensor

bool DS2401::duty(OneWireHub *hub)
{
    const uint8_t done = hub->recv();

    switch (done)
    {
        default:
            if (dbg_HINT)
            {
                Serial.print("DS2401=");
                Serial.println(done, HEX);
            }
            break;
    }
    return TRUE;
}
