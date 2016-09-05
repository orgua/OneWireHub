/*
 *    Example-Code that emulates a BAE910
 *    ( http://www.brain4home.eu/node/4 )
 *
 */

#include "OneWireHub.h"
#include "BAE910.h"  // 3rd party OneWire slave device, family code 0xFC

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;
const uint8_t Analog_PIN    = 0;

auto hub    = OneWireHub(OneWire_PIN);
auto bae910 = BAE910(BAE910::family_code, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06);


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
    Serial.println("OneWire-Hub BAE910 emulation ADC example");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(bae910);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.getError()) hub.printError();

    // write something into BAEs rtc
    bae910.memory.field.rtc = millis() / 1000;

    // Blink triggers the state-change
    if (blinking())
    {
        // read ADC and write into BAE register
        bae910.memory.field.adc10 = analogRead(Analog_PIN);
    }
}
