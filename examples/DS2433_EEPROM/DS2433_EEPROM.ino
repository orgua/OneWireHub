/*
*    Example-Code that emulates a DS2433 4096 bits EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2433.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub = OneWireHub(pin_onewire);
auto ds2433 = DS2433(DS2433::family_code, 0x01, 0x01, 0x33, 0x24, 0xD0, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2433");

    // Setup OneWire
    hub.attach(ds2433);

    char memory[] = "abcdefg-test-data directly from a texteditor - full ASCII:-?+";
    ds2433.writeMemory(memory,sizeof(memory),0x00);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 