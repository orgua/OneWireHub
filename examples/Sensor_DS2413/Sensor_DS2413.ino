/*
 *    Example-Code that emulates a DS2413 Dual channel addressable switch
 *    Tested with https://github.com/PaulStoffregen/OneWire --> still untested
 */

#include "OneWireHub.h"
#include "DS2413.h"  // Dual channel addressable switch

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

auto hub    = OneWireHub(OneWire_PIN);
auto ds2413 = DS2413( 0x3A, 0x00, 0x0D, 0x02, 0x04, 0x01, 0x03 );    // Work - Dual channel addressable switch

bool blinking()
{
    const  uint32_t interval    = 500;          // interval at which to blink (milliseconds)
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
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2413 Dual channel addressable switch");

    // Setup OneWire
    hub.attach(ds2413);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.waitForRequest(false);

    // Blink triggers the state-change
    if (blinking())
    {

    }
}