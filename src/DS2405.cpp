#include "DS2405.h"

DS2405::DS2405(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    pin_state = false;
}

void DS2405::duty(OneWireHub * const hub)
{
    // IC uses weird bus-features to operate., match-rom is enough
    pin_state = !pin_state;
    noInterrupts();
    while(!hub->sendBit(pin_state)); // if master issues read slots it gets the state...
    interrupts();

    // TODO: when alarm search is implemented (0xEC):
    // when PIO pin is driven low this device issues an alarm, otherwise stays alarm is disabled
}
