/*
 *    Example-Code that emulates a DS2413 Dual channel addressable switch
 *
 *    Tested with
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2413.h"  // Dual channel addressable switch

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2413 = DS2413( DS2413::family_code, 0x00, 0x00, 0x13, 0x24, 0xDA, 0x00 );    // Work - Dual channel addressable switch

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2413 Dual channel addressable switch");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2413);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test - set State of switch 0");
    Serial.println(ds2413.getPinState(0));
    ds2413.setPinState(0,1);
    Serial.println(ds2413.getPinState(0));

    Serial.println("Test - set State of switch 1");
    ds2413.setPinState(1,1);
    Serial.println(ds2413.getPinState(1));

    Serial.println("Test - set Latch of switch 1");
    Serial.println(ds2413.getPinLatch(1));
    ds2413.setPinLatch(1,1);
    Serial.println(ds2413.getPinLatch(1)); // latch is set

    Serial.println("Test - check State of switch 1");
    Serial.println(ds2413.getPinState(1)); // will be zero because of latching
    ds2413.setPinState(1,1);
    Serial.println(ds2413.getPinState(1)); // still zero

    Serial.println("Test - disable latch and set State of switch 1");
    ds2413.setPinLatch(1,0);
    ds2413.setPinState(1,1);
    Serial.println(ds2413.getPinState(1)); // works again, no latching

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        Serial.print(" A: ");
        Serial.print(ds2413.getPinState(0));
        Serial.print(" / ");
        Serial.print(ds2413.getPinLatch(0));
        Serial.print(" B: ");
        Serial.print(ds2413.getPinState(1));
        Serial.print(" / ");
        Serial.print(ds2413.getPinLatch(1));
        Serial.println(" (State / Latch)");
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 1000;          // interval at which to blink (milliseconds)
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
