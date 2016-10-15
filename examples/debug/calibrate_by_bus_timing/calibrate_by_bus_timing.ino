/*
 *    Test-Code for calibration - this sketch determines the value for "instructions per loop"
 *
 *      --> read value per serial-com and write it to /src/platform.h to your specific architecture
 *          >>>>  constexpr uint8_t VALUE_IPL {0}; // instructions per loop
 *
 *      --> test it with a normal sensor-sketch (like ds18b20_thermometer.ini)
 *
 *      --> if it works please make a pullrequest or open an issue to report your determined value
 *          >>>> https://github.com/orgua/OneWireHub
 */

#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

constexpr uint8_t pin_onewire   { 8 };

auto hub     = OneWireHub(pin_onewire,0); // do an bus-timing-calibration on first sensor-attachment
auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x02, 0x0B, 0x08, 0x01, 0x0D);    // Digital Thermometer

/////////////////////////////////////////////////////////////////////////
/////// Main Code              //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub calibration by observing the OW-Bus");
    Serial.flush();

    hub.attach(ds18b20);
};

void loop()
{
    const timeOW_t factor_ipl = hub.waitLoopsCalibrate();

    Serial.print(factor_ipl);
    Serial.println("\t instructions per loop");

    hub.poll();
}
