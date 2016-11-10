/*
 *    Example-Code that emulates a DS2502 - 1kbit EEPROM as a dell power supply
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 *    - DS9490R-Master, atmega328@16MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2502.h"

constexpr uint8_t pin_onewire   { 8 };

constexpr uint8_t charger130W[4] = {0xFB, 0x31, 0x33, 0x30};  //130W
constexpr uint8_t charger090W[4] = {0xFB, 0x30, 0x39, 0x30};  //90W
constexpr uint8_t charger065W[4] = {0xFB, 0x30, 0x36, 0x36};  //65W

auto hub        = OneWireHub(pin_onewire);
auto dellCHa    = DS2502( 0x89, 0x00, 0x00, 0xA0, 0x11, 0xDE, 0x00 ); // should be x28, but is not testable by the ds9490 this way
auto dellCHb    = DS2502( 0x28, 0x00, 0x00, 0xA0, 0x11, 0xDE, 0x00 ); // should work

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2502 aka Dell Charger");

    // Setup OneWire
    hub.attach(dellCHa);
    hub.attach(dellCHb);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Write 130W-Data into Power Supply and redirect every page to it");
    dellCHa.writeMemory(charger130W, sizeof(charger130W), 0x20); // write to bank 1
    dellCHa.setPageRedirection(0,1); // set memorybanks to all read from bank 1
    dellCHa.setPageRedirection(1,1);
    dellCHa.setPageRedirection(2,1);
    dellCHa.setPageRedirection(3,1);

    dellCHb.writeMemory(charger130W, sizeof(charger130W), 0x20); // write to bank 1
    dellCHb.setPageRedirection(0,1); // set memorybanks to all read from bank 1
    dellCHb.setPageRedirection(1,1);
    dellCHb.setPageRedirection(2,1);
    dellCHb.setPageRedirection(3,1);

    // dellCHb.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

}