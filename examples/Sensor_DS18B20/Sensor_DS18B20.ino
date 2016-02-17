/*
 *    Example-Code that emulates a DS18B20
 *    Tested with https://github.com/PaulStoffregen/OneWire --> DS18x20-Example
 */


#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

const uint8_t OneWire_PIN = 8;

OneWireHub  hub     = OneWireHub(OneWire_PIN);
auto        sensor  = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);    // Digital Thermometer

void setup()
{
    // Debug
    Serial.begin(115200);
    Serial.println("OneWire-Hub Temperature-Sensor DS18B20");

    // Setup OneWire
    hub.attach(sensor);

    Serial.println("config done");
}

void loop()
{
    // Set temp
    sensor.setTemp(21);

    // put your main code here, to run repeatedly:
    hub.waitForRequest();
}