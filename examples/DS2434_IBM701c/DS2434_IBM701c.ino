/*
*    Example-Code that emulates a DS2434 with Battery Management & EEPROM
*
*   Tested with
*    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2434.h"

constexpr uint8_t pin_onewire   { 2 };

auto hub = OneWireHub(pin_onewire);
auto ds2434 = DS2434(0x1B, 0x01, 0x02, 0x12, 0x34, 0x56, 0x78);

void setup()
{
    // Setup OneWire
    hub.attach(ds2434);

    // add default-data
    constexpr uint8_t mem1[24] = {0x14, 0x10, 0x90, 0xd0, 0x03, 0x32, 0x4b, 0x3c,
                                  0xff, 0x04, 0x64, 0x04, 0x9e, 0x9a, 0x3a, 0xf0,
                                  0x20, 0x20, 0x04, 0xee, 0x63, 0xB8, 0x3E, 0x63 };  // last 4 Byte seem to be Serial
    ds2434.writeMemory(reinterpret_cast<const uint8_t *>(mem1),sizeof(mem1),0x00);

    constexpr  uint8_t mem2[8] = {0x33, 0x2e, 0x33, 0x2e, 0x9e, 0x10, 0x3f, 0x50 };
    ds2434.writeMemory(reinterpret_cast<const uint8_t *>(mem2),sizeof(mem2),0x20);

    ds2434.lockNV1();
    ds2434.setID(0xCABDu);
    ds2434.setBatteryCounter(0x0123u);
    ds2434.setTemperature(22u);
}

void loop()
{
    static uint8_t temp = 20u;

    // following function must be called periodically
    hub.poll();

    // demo: every read increases the Temp
    if (ds2434.getTemperatureRequest())
    {
        ds2434.setTemperature(temp++);
        if (temp > 25u) temp = 20u;
    }

} 