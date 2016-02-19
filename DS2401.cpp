#include "OneWireHub.h"
#include "DS2401.h"

DS2401::DS2401(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
};

//DS2401::DS2401(uint8_t SER1, uint8_t SER2, uint8_t SER3, uint8_t SER4, uint8_t SER5, uint8_t SER6) : DS2401(family_code,SER1,SER2,SER3,SER4,SER5,SER6)
//{};

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
