/*
 *    Example-Code that emulates a DS2503 4kbit EEPROM, Add Only Memory
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
    Serial.println("OneWire-Hub DS2503");

    // Setup OneWire
    hub.attach(ds2503);
    hub.attach(ds2505);
    hub.attach(ds2506);

    Serial.println("Test Redirection Protection");
    Serial.println(ds2506.getRedirectionProtection(2));
    ds2506.setRedirectionProtection(2);
    Serial.println(ds2506.getRedirectionProtection(1));
    Serial.println(ds2506.getRedirectionProtection(2));
    Serial.println(ds2506.getRedirectionProtection(3));

    Serial.println("Test Page Redirection");
    Serial.println(ds2506.getPageRedirection(3));
    ds2506.setPageRedirection(3,4);
    Serial.println(ds2506.getPageRedirection(3));

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

}