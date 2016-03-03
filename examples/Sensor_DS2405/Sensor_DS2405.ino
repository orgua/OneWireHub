/*
 *    Example-Code that emulates a DS2405 - One Channel addressable switch
 *
 *    Tested with
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2405.h"  // Dual channel addressable switch

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

auto hub    = OneWireHub(OneWire_PIN);
auto ds2405 = DS2405( DS2405::family_code, 0x00, 0x0D, 0x02, 0x04, 0x00, 0x05 );    // Work - Dual channel addressable switch

bool blinking()
{
    const  uint32_t interval    = 1000;          // interval at which to blink (milliseconds)
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
    Serial.println("OneWire-Hub DS2405 - One channel addressable switch");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(ds2405);
    ds2405.setState(1);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        Serial.print(" PinState: ");
        Serial.println(ds2405.readState());
    }
}