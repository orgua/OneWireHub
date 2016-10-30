/*
*   Example-Code that emulates a DS2450 4 channel A/D
*	
*	not fully implemented yet! Only basic Memory-Reading
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2450.h"

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2450 = DS2450( DS2450::family_code, 0x00, 0x00, 0x50, 0x24, 0xDA, 0x00 );

bool blinking()
{
    const  uint32_t interval    = 2000;          // interval at which to blink (milliseconds)
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

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2450 4 channel A/D");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2450);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        static uint16_t poti_value = 512;
        poti_value += 8;
        ds2450.setPotentiometer(poti_value, poti_value + 512, poti_value + 1024, poti_value + 2048);
    }
} 