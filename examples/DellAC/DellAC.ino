#include "OneWireHub.h"
#include "DellAC.h"

const uint8_t OneWire_PIN   = 9;
auto hub     = OneWireHub(OneWire_PIN);
auto dellACSpoofer = DellAC(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x0A); 

void setup(void)
{
  hub.attach(dellACSpoofer);
}

void loop() {
  hub.poll();
}
