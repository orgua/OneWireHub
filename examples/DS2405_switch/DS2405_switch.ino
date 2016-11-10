/*
 *    Example-Code that emulates a DS2405 - One Channel addressable switch
 *
 *    Tested with
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 *    - Note: feedback to master is unclear, ds9490 does not read after matchRom
 */

#include "OneWireHub.h"
#include "DS2405.h"  // Dual channel addressable switch

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2405 = DS2405( DS2405::family_code, 0x00, 0x00, 0x05, 0x24, 0xDA, 0x00 );    // Work - Dual channel addressable switch

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2405 - One channel addressable switch");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2405);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test - clear State of GPIO 0");
    Serial.println(ds2405.getPinState());
    ds2405.setPinState(1);
    Serial.println(ds2405.getPinState());

    Serial.println("config done");
}

void loop()
{
    static bool switch_state = 0;

    // following function must be called periodically
    hub.poll();

    if (switch_state != ds2405.getPinState())
    {
        switch_state = ds2405.getPinState();

        // visual feedback and printF
        Serial.print(" PinState: ");
        Serial.println(switch_state);

        digitalWrite(pin_led, switch_state);
    }


}