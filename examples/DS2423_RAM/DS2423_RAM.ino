/*
*    Example-Code that emulates a DS2423 4096 bits RAM with Counter
*
*   THIS DEVICE IS ONLY A MOCKUP FOR NOW - NO REAL FUNCTION
*
*   Tested with
*   - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2423.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub = OneWireHub(pin_onewire);
auto ds2423 = DS2423(DS2423::family_code, 0x00, 0x00, 0x23, 0x24, 0xDA, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2423");

    // Setup OneWire
    hub.attach(ds2423);

    // Test-Cases: the following code is just to show basic functions, can be removed any time


    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 