/*
 *    Example-Code that emulates a BAE910
 *    ( http://www.brain4home.eu/node/4 )
 *
 *    Tested with:
 */

#include "OneWireHub.h"
#include "BAE910.h"  // 3rd party OneWire slave device, family code 0xFC

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };
constexpr uint8_t pin_analog    { 0 };

auto hub    = OneWireHub(pin_onewire);
auto bae910 = BAE910(BAE910::family_code, 0x00, 0x00, 0x10, 0xE9, 0xBA, 0x00);

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub BAE910 emulation ADC example");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(bae910);
    bae910.memory.field.SW_VER = 0x01;
    bae910.memory.field.BOOTSTRAP_VER = 0x01;

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.hasError()) hub.printError();

    // write something into BAEs rtc
    bae910.memory.field.rtc = millis() / 1000;

    // Blink triggers the state-change
    if (blinking())
    {
        // read ADC and write into BAE register
        bae910.memory.field.adc10 = analogRead(pin_analog);
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 500;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}
