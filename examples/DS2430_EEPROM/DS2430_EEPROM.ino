/*
*    Example-Code that emulates a DS2430 1024 bits EEPROM
*
*   Tested with
*    - DS9490R-Controller, atmega328@16MHz as device-emulator
*/

#include "DS2430.h"
#include "OneWireHub.h"

constexpr uint8_t pin_onewire{2};

auto hub    = OneWireHub(pin_onewire);
auto ds2430 = DS2430(DS2430::family_code, 0x00, 0x00, 0x31, 0x24, 0xDA, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2430");

    // Setup OneWire
    hub.attach(ds2430);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Write Text Data to page 0");
    constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
    ds2430.writeMemory(reinterpret_cast<const uint8_t *>(memory), sizeof(memory), 0x00);
    ds2430.syncScratchpad();

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
}
