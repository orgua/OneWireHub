/*
 *    Test-Code for a suitable timing function
 *
 *      --> try to determine bus-timing by observing the master
 *
 */

#include "OneWireHub.h"

constexpr uint8_t pin_led       { 13 }; // TODO: take this code to other examples
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);

/////////////////////////////////////////////////////////////////////////
/////// Main Code              //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void setup()
{
    pinMode(pin_led, OUTPUT);
    Serial.begin(115200);

    const uint8_t factor_ipl = hub.waitLoopsCalibrate();

    Serial.print(factor_ipl);
    Serial.println("\t instructions per loop");
};

void loop()
{

}
