/*
*    Example-Code that emulates a DS2433 4096 bits EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2433.h"

const uint8_t led_PIN = 13;         // the number of the LED pin
const uint8_t OneWire_PIN = 8;

auto hub = OneWireHub(OneWire_PIN);
auto ds2433 = DS2433(DS2433::family_code, 0x01, 0x01, 0x33, 0x24, 0xD0, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2433");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(ds2433);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 