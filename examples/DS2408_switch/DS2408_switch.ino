/*
 *    Example-Code that emulates a DS2408 - an 8CH GPIO Port Extender
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire on the other side as Master
 */

#include "OneWireHub.h"
#include "DS2408.h"

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

auto hub    = OneWireHub(OneWire_PIN);
auto ds2408 = DS2408( 0x29, 0x0D, 0x02, 0x04, 0x00, 0x08, 0x0A );


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
    Serial.println("OneWire-Hub DS2408 - 8CH GPIO Port Extender");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(ds2408); // always online

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.getError()) hub.printError();

    // Blink triggers the state-change
    if (blinking())
    {

    }
}