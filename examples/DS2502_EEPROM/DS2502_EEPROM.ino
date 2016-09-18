/*
 *    Example-Code that emulates a dell power supply
 *
 *    Tested with
 *    - dell notebook https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 */

#include "OneWireHub.h"
#include "DS2502.h"

const uint8_t led_PIN       = 8;         // the number of the LED pin
const uint8_t OneWire_PIN   = 1;

const uint8_t chargerData[4] = {0xFB, 0x31, 0x33, 0x30};//130W
//const uint8_t chargerData[4] = {0xFB, 0x30, 0x39, 0x30};//90W
//const uint8_t chargerData[4] = {0xFB, 0x30, 0x36, 0x36};//65W

auto hub     = OneWireHub(OneWire_PIN);
auto ds2502 = DS2502( DS2502::family_code, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A );

void setup()
{
    //Serial.begin(115200);
    //Serial.println("OneWire-Hub DS2502");

    pinMode(led_PIN, OUTPUT);

    // Setup OneWire
    ds2502.writeMemory(chargerData, sizeof(chargerData), 0);

    hub.attach(ds2502); // always online

    //Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

}