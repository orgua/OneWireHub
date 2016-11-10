/*
 *    Example-Code that emulates a DS2401 used as a binary button (like reed-contact - power on, power off)
 *
 *    Tested with
 *    - https://github.com/PaulStoffregen/OneWire on the other side as Master, atmega328@16MHz as Slave
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2401.h"  // Serial Number

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub     = OneWireHub(pin_onewire);
auto ds2401A = DS2401( DS2401::family_code, 0x00, 0xA0, 0x01, 0x24, 0xDA, 0x00 );    // Work - Serial Number
auto ds2401B = DS2401( DS2401::family_code, 0x00, 0xB0, 0x01, 0x24, 0xDA, 0x00 );    // Work - Serial Number
auto ds2401C = DS2401( DS2401::family_code, 0x00, 0xC0, 0x01, 0x24, 0xDA, 0x00 );
auto ds1990A = DS2401( 0x81, 0x00, 0xA0, 0x90, 0x19, 0xDA, 0x00 );

bool blinking(void);


void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2401 Serial Number used as iButton");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2401C); // always online
    hub.attach(ds1990A); // always online

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    // ds2401A and B alternate with each LED-Blink-Change, so there is only one online at a time
    // ds2401C is always online

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        static bool flipFlop = 0;
        // Change between Sensor A and B every 50 seconds
        if (flipFlop)
        {
            flipFlop = 0;
            hub.detach(ds2401A);
            hub.attach(ds2401B);
            Serial.println("B is active");
        }
        else
        {
            flipFlop = 1;
            hub.detach(ds2401B);
            hub.attach(ds2401A);
            Serial.println("A is active");
        }
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 50000;          // interval at which to blink (milliseconds)
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
