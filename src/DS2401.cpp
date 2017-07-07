#include "DS2401.h"

DS2401::DS2401(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

void DS2401::duty(OneWireHub * const hub)
{
    uint8_t cmd;

    if (hub->recv(&cmd))  return;

    switch (cmd)
    {
        default:

            hub->raiseSlaveError(cmd);
    }
}
