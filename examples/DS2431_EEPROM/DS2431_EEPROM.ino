/*
*    Example-Code that emulates a DS2431 1024 bits EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2431.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub = OneWireHub(pin_onewire);
auto ds2431 = DS2431(DS2431::family_code, 0xE8, 0x9F, 0x90, 0x0E, 0x00, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2431");

    // Setup OneWire
    hub.attach(ds2431);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 