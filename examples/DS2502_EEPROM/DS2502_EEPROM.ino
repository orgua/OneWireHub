/*
 *    Example-Code that emulates a dell power supply
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2502.h"

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

const uint8_t chargerData[4] = {0xFB, 0x31, 0x33, 0x30};//130W
//const uint8_t chargerData[4] = {0xFB, 0x30, 0x39, 0x30};//90W
//const uint8_t chargerData[4] = {0xFB, 0x30, 0x36, 0x36};//65W

auto hub        = OneWireHub(OneWire_PIN);
auto ds2502     = DS2502( DS2502::family_code, 0x02, 0x00, 0x05, 0x02, 0x0D, 0x00 );
auto ds2501a    = DS2502( 0x91, 0x01, 0x00, 0x05, 0x02, 0x0D, 0x00 );
auto ds2501b    = DS2502( 0x11, 0x01, 0x00, 0x05, 0x02, 0x0D, 0x00 );
auto dellCHa    = DS2502( 0x89, 0x0E, 0x01, 0x01, 0x0E, 0x0D, 0x00 ); // should be x28, but is not testable by the ds9490 this way
auto dellCHb    = DS2502( 0x28, 0x0E, 0x01, 0x01, 0x0E, 0x0D, 0x00 ); // should work

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2502");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    dellCHa.writeMemory(chargerData, sizeof(chargerData), 0x20); // write to bank 1
    dellCHa.redirectPage(0,1); // set memorybanks to all read from bank 1
    dellCHa.redirectPage(1,1);
    dellCHa.redirectPage(2,1);
    dellCHa.redirectPage(3,1);

    dellCHb.writeMemory(chargerData, sizeof(chargerData), 0x20); // write to bank 1
    dellCHb.redirectPage(0,1); // set memorybanks to all read from bank 1
    dellCHb.redirectPage(1,1);
    dellCHb.redirectPage(2,1);
    dellCHb.redirectPage(3,1);

    hub.attach(ds2502);
    hub.attach(ds2501a);
    hub.attach(ds2501b);
    hub.attach(dellCHa);
    hub.attach(dellCHb);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

}