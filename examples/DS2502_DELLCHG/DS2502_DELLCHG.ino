/*
 *    Example-Code that emulates a DS2502 - 1kbit EEPROM as a dell power supply
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 *    - DS9490R-OneWire-Host, atmega328@16MHz as peripheral device
 *    - Arduino ProMini clone
 *    - esp8266 (ESP-01 module, using GPIO2 with a 10k pull-up resistor)
 *    - Dell Inspiron 15R N5110 and Dell Inspiron 15R 5521 @130W
 *
 *    OneWire messaging starts when AC adapter is plugged to notebook,
 *    try to use parasite powering but unfortunately it doesn't provide enough power,
 *    so You need DC-DC converter to power MCU
 *    Linear regulators can only be used as a temporary replacement, e.g., a combination of LM7805 and AMS1117.
 *    During the test, the temperature of the LM7805 reaches 60 celsius when powering an ESP-01 module.
 *
 *    thanks to Nik / ploys for supplying traces of real data-traffic to figure out communication:
 *    - reset and presence detection normal
 *    - cmd from host: 0xCC -> skip rom, so there is only ONE device allowed on the bus
 *    - cmd from host: 0xF0 -> read memory
 *    - address request from host: 0x0008
 *    - OneWire-Host listens for data, gets CRC of seconds cmd and address first, then listens for 3 bytes, does not listen any further
 *    !!! Note that some latest Dell models may ask for more information !!!
 */

#include "DS2502.h"
#include "OneWireHub.h"

// Using GPIO2 on an ESP01 module (Requires 10k pull-up to 3.3V)
constexpr uint8_t pin_onewire{2};

// EEPROM strings, the length is always 42 bytes, including 2 bytes of CRC16 checksum.
constexpr uint8_t     chargerStrlen{42};
// https://github.com/KivApple/dell-charger-emulator
constexpr const char *charger45W  = "DELL00AC045195023CN0CDF577243865Q27F2A05\x3D\x94";
// https://nickschicht.wordpress.com/2009/07/15/dell-power-supply-fault/
constexpr const char *charger65W  = "DELL00AC065195033CN05U0927161552F31B8A03\xBC\x8F";
constexpr const char *charger90W  = "DELL00AC090195046CN0C80234866161R23H8A03\x4D\x7C";
// I made this up, works with Dell Inspiron 15R N5110 and Dell Inspiron 15R 5521
constexpr const char *charger130W = "DELL00AC130195067CN0CDF577243865Q27F2233\x9D\x72";
// 240W tested on Dell 9380 via Dell TB16 dock
constexpr const char* charger240W = "DELL00AC240195123CN0FWCRC4866165H3M9PA05\x9F\xA3";


auto hub    = OneWireHub(pin_onewire);
auto dellCH = DS2502(
        0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02,
        0x0A); // address does not matter, laptop uses skipRom -> note that therefore only one peripheral device is allowed on the bus

void setup()
{
    //Serial.begin(115200);
    //Serial.println("OneWire-Hub DS2502 aka Dell Charger");

    // Setup OneWire
    hub.attach(dellCH);
    // Populate the emulated EEPROM with the 42 byte ID string
    dellCH.writeMemory((uint8_t *) charger130W, chargerStrlen);
}

void loop()
{
    // following function must be called periodically
    hub.poll();
}
