/* 
 * Working example for Dell AC spoofing
 * Thanks to Kondi from https://forum.pjrc.com/threads/33640-Teensy-2-OneWire-Slave
 * Tested on Arduino ProMini and esp8266 boards, works well
 *
 * OneWire messaging starts when AC adapter is plugged to notebook, 
 * try to use parasite powering but unfortunately it doesn't provide enough power,
 * so You need DC-DC converter to power MCU
 */
#include "OneWireHub.h"
#include "DellAC.h"

const uint8_t OneWire_PIN   = 9;
auto hub = OneWireHub(OneWire_PIN);
auto dellACSpoofer = DellAC(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A); 

void setup(void) {
  hub.attach(dellACSpoofer);
}

void loop() {
  hub.poll();
}
