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
auto ds2433 = DS2433(DS2433::family_code, 0x00, 0x00, 0x33, 0x24, 0xDA, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2433");

    // Setup OneWire
    hub.attach(ds2433);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Write Text Data to page 0");
    constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
    ds2433.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),0x00);

    Serial.println("Test Write binary Data to page 1");
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2433.writeMemory(mem_dummy, sizeof(mem_dummy), 1*32);

    Serial.print("Test Read binary Data to page 1: 0x");
    uint8_t mem_read[16];
    ds2433.readMemory(mem_read, 16, 31); // begin one byte earlier than page 1
    Serial.println(mem_read[2],HEX); // should read 0x11

    // ds2433.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 