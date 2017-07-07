/*
 *    sketch for code size comparison
 *
 *    track program-size development
 *
 *    7350 // 470 bytes v1.1.0 without gpio- and serial-debug (basic ds18b20-thermometer-sketch)
 *    5280 // 439 bytes without blinking-code and float-temp-operations
 *    4222 // 212 bytes without serial
 *    3856 // 172 bytes just 1 instead of 3 ds18b20
 *
 *    5150 // 181 bytes switch to alternative branch with automatic timing calibration
 *
 *    4504 // 301 bytes switch to new branch with static calibration
 *    4542 // 301 bytes get rid of wait() and delayMicroseconds()
 *    4430 // 301 bytes get rid of micros()
 *    4388 // 301 bytes clean awaitTimeSlotAndWrite() and debug-pin-access
 *    4404 // 300 bytes add resume-cmd
 *    4362 // 300 bytes return true on error for send(), awaitTS(), sendBit(), recv([])
 *    4388 // 300 bytes make skipRom and readRom more safe
 *    3440 // 128 bytes rework error-handling and send/recv-routines
 */

#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);

auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x02, 0x0B, 0x08, 0x01, 0x0D);    // Digital Thermometer


void setup()
{
    // Setup OneWire
    hub.attach(ds18b20);

    // Set const temperature
    const int8_t temperature = 21;
    ds18b20.setTemperature(temperature);
}

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.hasError()) hub.printError();
}