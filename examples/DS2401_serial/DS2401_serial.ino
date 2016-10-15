/*
 *    Example-Code that emulates a DS2401 used as a binary button (like reed-contact - power on, power off)
 *
 *    Tested with
 *    - https://github.com/PaulStoffregen/OneWire on the other side as Master, atmega328@16MHz as Slave
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2401.h"  // Serial Number

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub     = OneWireHub(pin_onewire);
auto ds2401A = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0A );    // Work - Serial Number
auto ds2401B = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0B );    // Work - Serial Number
auto ds2401C = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0C );

bool blinking()
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


void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2401 Serial Number used as iButton");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2401C); // always online

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