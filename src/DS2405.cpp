#include "DS2405.h"

DS2405::DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    pin_state = 0;
};

bool DS2405::duty(OneWireHub *hub)
{
    // IC uses weird bus-features to operate.

    pin_state = !pin_state;
    hub->sendBit(pin_state);

    return !(hub->getError());
};

