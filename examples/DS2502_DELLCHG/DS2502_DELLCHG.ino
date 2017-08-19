/*
 *    Example-Code that emulates a DS2502 - 1kbit EEPROM as a dell power supply
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 *    - DS9490R-Master, atmega328@16MHz as Slave
 *    - Arduino ProMini clone
 *    - esp8266
 *
 *    OneWire messaging starts when AC adapter is plugged to notebook,
 *    try to use parasite powering but unfortunately it doesn't provide enough power,
 *    so You need DC-DC converter to power MCU
 *
 *    thanks to Nik / ploys for supplying traces of real data-traffic to figure out communication:
 *    - reset and presence detection normal
 *    - cmd from master: 0xCC -> skip rom, so there is only ONE device allowed on the bus
 *    - cmd from master: 0xF0 -> read memory
 *    - address request from master: 0x0008
 *    - master listens for data, gets CRC of seconds cmd and address first, then listens for 3 bytes, does not listen any further
 */

#include "OneWireHub.h"
#include "DS2502.h"

constexpr uint8_t pin_onewire   { 9 };

constexpr uint8_t charger130W[4] = {0x31, 0x33, 0x30};  //130W (=second digit of each hex-number)
constexpr uint8_t charger090W[4] = {0x30, 0x39, 0x30};  //90W
constexpr uint8_t charger065W[4] = {0x30, 0x36, 0x36};  //66W

auto hub       = OneWireHub(pin_onewire);
auto dellCH    = DS2502( 0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A); // address does not matter, laptop uses skipRom -> note that therefore only one slave device is allowed on the bus

void setup()
{
    //Serial.begin(115200);
    //Serial.println("OneWire-Hub DS2502 aka Dell Charger");

    // Setup OneWire
    hub.attach(dellCH);

    dellCH.writeMemory(charger130W, sizeof(charger130W), 0x08);
}

void loop()
{
    // following function must be called periodically
    hub.poll();
}