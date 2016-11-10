/*
 *    Example-Code that emulates a DS2502 - 1kbit EEPROM, Add Only Memory
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2502.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub        = OneWireHub(pin_onewire);
auto ds2502     = DS2502( DS2502::family_code, 0x00, 0xA0, 0x02, 0x25, 0xDA, 0x00 );
auto ds2501a    = DS2502( 0x91, 0x00, 0xA0, 0x01, 0x25, 0xDA, 0x00 );
auto ds2501b    = DS2502( 0x11, 0x00, 0xB0, 0x02, 0x25, 0xDA, 0x00 );

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2502");

    // Setup OneWire
    hub.attach(ds2502);
    hub.attach(ds2501a);
    hub.attach(ds2501b);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Page Redirection p1 to p3");
    Serial.println(ds2502.getPageRedirection(1));
    ds2502.setPageRedirection(1,3);
    Serial.println(ds2502.getPageRedirection(1));

    Serial.println("Test Write Data to page 3");
    Serial.println(ds2502.getPageUsed(3));
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2502.writeMemory(mem_dummy, sizeof(mem_dummy), 3*32);
    Serial.println(ds2502.getPageUsed(3));

    Serial.println("Test Write Data to protected page 0 -> is possible, only affects master");
    Serial.println(ds2502.getPageUsed(0));
    Serial.println(ds2502.getPageProtection(0));
    ds2502.setPageProtection(0);
    Serial.println(ds2502.getPageProtection(0));
    ds2502.writeMemory(mem_dummy, sizeof(mem_dummy), 16); // write in second half of page
    Serial.println(ds2502.getPageUsed(0));

    Serial.print("Test Read binary Data to page 3: 0x");
    uint8_t mem_read[16];
    ds2502.readMemory(mem_read, 16, 3*32 - 1); // begin one byte earlier than page 4
    Serial.println(mem_read[2],HEX); // should read 0x11

    // ds2502.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
}
