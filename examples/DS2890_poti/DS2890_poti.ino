/*
 *    Example-Code that emulates a DS2890 Single channel digital potentiometer (datasheet has it covered for up to 4 CH)
 *
 *    Tested with
 *    - https://github.com/PaulStoffregen/OneWire --> still untested
 */

#include "OneWireHub.h"
#include "DS2890.h"  // Single channel digital potentiometer

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

auto hub    = OneWireHub(OneWire_PIN);
auto ds2890 = DS2890( 0x2C, 0x00, 0x0D, 0x02, 0x08, 0x09, 0x00 );    // Work - Single channel digital potentiometer

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
    Serial.println("OneWire-Hub DS2890 Single channel digital potentiometer");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(ds2890);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        Serial.print("Potentiometer: ");
        Serial.print(ds2890.readPoti(0));
        Serial.print(" ");
        Serial.print(ds2890.readPoti(1));
        Serial.print(" ");
        Serial.print(ds2890.readPoti(2));
        Serial.print(" ");
        Serial.print(ds2890.readPoti(3));
        Serial.print(" of 255 with config: ");
        Serial.println(ds2890.readCtrl());

    }
}