/*
 *    Test-Code for a suitable timing function
 *
 *      --> try to determine bus-timing by observing the master
 *
 */

#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

constexpr uint8_t pin_led       { 13 }; // TODO: take this code to other examples
constexpr uint8_t pin_onewire   { 8 };

auto hub     = OneWireHub(pin_onewire,0);
auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x02, 0x0B, 0x08, 0x01, 0x0D);    // Digital Thermometer

/////////////////////////////////////////////////////////////////////////
/////// Main Code              //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void setup()
{
    pinMode(pin_led, OUTPUT);
    Serial.begin(115200);
    hub.attach(ds18b20);
};

void loop()
{
    const timeOW_t factor_ipl = hub.waitLoopsCalibrate();

    Serial.print(factor_ipl);
    Serial.println("\t instructions per loop");
    hub.debugTiming();
    hub.poll();
}
