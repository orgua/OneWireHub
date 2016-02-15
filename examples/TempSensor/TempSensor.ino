#include "OneWireHub.h"
#include "DS18B20.h"  // Dual channel addressable switch

const uint8_t OneWire_PIN = 8;

OneWireHub  hub = OneWireHub(OneWire_PIN);
DS18B20     fMS = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);    // Work - Digital Thermometer

void setup()
{
    // Debug
    Serial.begin(115200);
    Serial.println("OneWire-TempSensor DS18B20");

    // Setup OneWire
    hub.elms[0] = &fMS;
    hub.calck_mask();
}

void loop()
{
    // Set temp
    fMS.settemp(static_cast<float>(20.1));

    // put your main code here, to run repeatedly:
    hub.waitForRequest(false);
}