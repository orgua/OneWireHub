/*
 *    Example-Code that emulates 4 sensors (2x ds2401, 2x ds18b20)
 *    --> attach sensors as needed
 *
 *    Tested with:
 *    - Attiny85-Board http://www.ebay.de/itm/221702922456
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"

// include all libs to find errors
#include "DS2401.h"  // Serial Number
#include "DS18B20.h" // Digital Thermometer

const uint8_t led_PIN       = 1;         // the number of the LED pin
const uint8_t OneWire_PIN   = 2;

OneWireHub  hub      = OneWireHub(OneWire_PIN);
DS18B20     ds18B20a = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A);     // Work - Digital Thermometer
DS18B20     ds18B20b = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0B);
DS2401      ds2401a  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0A );    // Work - Serial Number
DS2401      ds2401b  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0B );


bool blinking()
{
    const  uint32_t interval    = 5000;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(led_PIN, ledState);
        return 1;
    }
    return 0;
}

void setup()
{
    pinMode(led_PIN, OUTPUT);
    // Setup OneWire
    const int8_t temperature_degC = 10;
    ds18B20a.setTemperature(temperature_degC);
    hub.attach(ds18B20a);
    hub.attach(ds18B20b);
    hub.attach(ds2401a);
    hub.attach(ds2401b);
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        static int8_t temperature_degC = 20;
        temperature_degC += 1;
        if (temperature_degC > 40) temperature_degC = 10;
        ds18B20b.setTemperature(temperature_degC);
    }
}
