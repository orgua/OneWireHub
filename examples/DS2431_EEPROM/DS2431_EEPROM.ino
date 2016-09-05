/*
*    Example-Code that emulates a DS2431 1024 bits EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2431.h"

const uint8_t led_PIN = 13;         // the number of the LED pin
const uint8_t OneWire_PIN = 8;

auto hub = OneWireHub(OneWire_PIN);
auto ds2431 = DS2431(DS2431::family_code, 0xE8, 0x9F, 0x90, 0x0E, 0x00, 0x00);
void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2431");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    hub.attach(ds2431);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 