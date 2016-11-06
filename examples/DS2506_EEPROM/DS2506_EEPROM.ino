/*
 *    Example-Code that emulates a DS2506 4kbit EEPROM, Add Only Memory
 *
 *      DOES NOT WORK YET
 *
 *    Tested with

 */

#include "OneWireHub.h"
#include "DS2506.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub        = OneWireHub(pin_onewire);

auto ds2503     = DS2506( 0x13, 0x00, 0x00, 0x03, 0x25, 0xDA, 0x00 );
auto ds2505     = DS2506( 0x0B, 0x00, 0x00, 0x05, 0x25, 0xDA, 0x00 );
auto ds2506     = DS2506( 0x0F, 0x00, 0x00, 0x06, 0x25, 0xDA, 0x00 );


void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2506");

    // Setup OneWire
    hub.attach(ds2503);
    hub.attach(ds2505);
    hub.attach(ds2506);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Redirection Protection for page 2");
    Serial.println(ds2506.getRedirectionProtection(2)); // Zero
    ds2506.setRedirectionProtection(2);
    Serial.println(ds2506.getRedirectionProtection(1)); // Zero
    Serial.println(ds2506.getRedirectionProtection(2)); // is now protected : ONE
    Serial.println(ds2506.getRedirectionProtection(3)); // Zero

    Serial.println("Test Page Redirection p3 to p4");
    Serial.println(ds2506.getPageRedirection(3));
    ds2506.setPageRedirection(3,4);
    Serial.println(ds2506.getPageRedirection(3));

    Serial.println("Test Page Redirection p2 to p4 -> fails");
    Serial.println(ds2506.getPageRedirection(2));
    ds2506.setPageRedirection(2,4);
    Serial.println(ds2506.getPageRedirection(2)); // should show the same as before --> is protected

    Serial.println("Test Write Data to page 4");
    Serial.println(ds2506.getPageUsed(4)); // unused
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2506.writeMemory(mem_dummy, sizeof(mem_dummy), 4*32);
    Serial.println(ds2506.getPageUsed(4)); // is used now

    Serial.println("Test Write Data to protected page 0 -> is possible, only affects master");
    Serial.println(ds2506.getPageUsed(0));        // is unused
    Serial.println(ds2506.getPageProtection(0));  // is unprotected
    ds2506.setPageProtection(0);                  // protect it
    Serial.println(ds2506.getPageProtection(0));  // is protected
    ds2506.writeMemory(mem_dummy, sizeof(mem_dummy), 16); // write in second half of page
    Serial.println(ds2506.getPageUsed(0));        // is used now

    Serial.print("Test Read binary Data to page 4: 0x");
    uint8_t mem_read[16];
    ds2506.readMemory(mem_read, 16, 4*32 - 1); // begin one byte earlier than page 4
    Serial.println(mem_read[2],HEX); // should read 0x11

    // ds2506.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

}