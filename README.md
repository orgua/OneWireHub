OneWireHub
==========

The OneWireHub is an Arduino compatible (and many more platforms) library to emulate OneWire-Slaves with support for various devices. The motivation is to offer a shared code base for all OneWire-Slaves. With a small overhead one µC can emulate up to 32 ICs simultaneously. 
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your environment.

### Supported Slaves:
- **BAE910 (0xFC) multi purpose device (ADC, Clock, GPIO, PWM, EEPROM)**
- **DS1822 (0x22) Digital Thermometer, 12bit** (use DS18B20 with different family code)
- **DS18B20 (0x28) Digital Thermometer, 12bit** (also known as DS1820) 
- **DS18S20 (0x10) Digital Thermometer, 12bit** (also known as DS1920, use DS18B20 with different family code)
- **DS1990 (0x01) iButton** (DS2401 with same family code)
- **DS2401 (0x01) Serial Number**
- **DS2405 (0x05) Single address switch**
- DS2408 (0x29) 8-Channel Addressable Switch, GPIO Port-expander
- **DS2411 (0x01) Serial Number** (use DS2401 with same family code)
- **DS2413 (0x3A) Dual channel addressable switch with input-sensing**
- DS2423 (0x1D) 4kbit 1-Wire RAM with Counter
- **DS2431 (0x2D) 1kbit protected EEPROM** (also known as DS1972 or DS28E07, same FC)
- DS2432 (0x33) 1kbit protected EEPROM (basically a ds2431 with extra sha-engine)
- **DS2433 (0x23) 4Kbit 1-Wire EEPROM**
- **DS2438 (0x26) Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit**
- DS2450 (0x20) 4 channel A/D
- **DS2501 (0x11, 0x91) 512bit EEPROM** (use DS2502 with different family code)
- **DS2502 (0x09, 0x89) 1kbit EEPROM, Add Only Memory** (also known as DS1982, same FC)
- **DS2890 (0x2C) 0x Single channel digital potentiometer - extended to 1-4 CH**
- Dell Power Supply (use DS2502 with family code set to 0x28)

Note: **Bold printed devices are feature-complete and were mostly tested with a DS9490 (look into the regarding example-file for more information)**

### Features:
- supports up to 32 slaves (8 is standard setting), adjust HUB_SLAVE_LIMIT in OneWireHub.h to safe RAM & program space
- hot-plug slaves as needed
- cleaner, faster code with c++11 features **(requires arduino sw 1.6.x or higher, >=1.6.10 recommended)**
   - i.e. use of constexpr instead of #define for better compiler-messages
- hardware-dependencies are combined in "platform.h", synced with [onewire-lib](https://github.com/PaulStoffregen/OneWire)
   - extra supported: arduino zero, teensy, sam3x, pic32, [ATtiny](https://github.com/damellis/attiny), esp8266, nrf51822 (...)
   - for portability and tests the hub can even be compiled on a PC with the supplied mock-up functions
   - at the moment the lib relies sole on the micros()-fn for timing, no direct access to interrupt or timers
- Serial debug output can be enabled in OneWireHub.h: set USE_SERIAL_DEBUG to 1 (be aware! it may produce heisenbugs, timing is critical)
- documentation, numerous examples, easy interface for hub and sensors

### Recent development (latest at the top):
- fix and clean pin access, fix a portability issue (time_t)
- prepare hub for overdrive-mode
- added or extended the ds2431, ds2431, ds2501, ds2502 (also tested)
- hub is more resilient to odd master-behaviour (lazy timings and subsequent resets are handled now), extended in 0.9.3 and 0.9.4
- added ds2431 (thanks to j-langlois) and BAE910 (thanks to Giermann), Dell Power Supply (thanks to Kondi)
- prepare new timing-method which will replace the old one in the next couple of weeks (a 6µs millis() call at 8MHz is not suitable for OW) 
- cleanup send / receive / waitForTimeslot to react faster to bus (better for µC with less than 16 MHz)
- support for skipROM-cmd if only one slave is present (thanks to Giermann)
- speed up atmel-crc-functions
- tested with DS9490R: ds28b20, ds2401, ds2405, ds2413, more will follow
- rework of error system, switch to enum, slaves can raise errors now & and Serial interferes less with OW-timings
- rework of the whole timings, if needed you can configure overdrive speed (arduino uno would probably be to slow)
- bug fix: non conformal behaviour as a onewire-slave (hopefully)
- raise the maximal slave limit from 8 to 32, code adapts via variable dataTypes
- open up for a lot more platforms with "platform.h" (taken from onewire-lib)
- per-bit-CRC16 with sendAndCRC16() and sendAndCRC16() for load-balancing, 900ns/bit instead of 7µs/byte on Atmega328@16MHz
- add examples for onewire-master, for testing the bus
- rework of checkReset(), showPresence(), send(), recv() - Hub is much more reliable now and it saves ~120 byte program-space
- faster CRC16 (ds2450 and ds2408 and ds2423), takes 5-7µs/byte instead of 10µs
- refactored the interface: hub.poll() replaces hub.waitForRequest()
- extended ds2890 to up to 4CH (datasheet has it covered), ds2413, ds2413 --> feature-complete
- implement and test ds2438
- replace search() algorithm, safes a lot of ram (debug-codeSize-4slaves.ino needs 3986 & 155 byte instead of 3928 & 891 byte) and allows >4 devices

### Plans for the future:
- implementation of ds2450
- ~~add table of tested and working sensors~~ (documented in the examples of the device)
- introduce unittests
- irq-handled hub on supported ports, split lib into onewire() and onewireIRQ()
- test each example with real onewire-masters, for now it's tested with the onewire-lib and a loxone-system (ds18b20 passed)
- ~~DS1963S 0x18 iButton, datasheet under NDA~~
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)
- [List of Maxim Sensors](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3989) (at the bottom)

### Connecting the HUB with the Network: 

![Onewire-Schematic](http://wiki.lvl1.org/images/1/15/Onewire.gif)

[read more](http://wiki.lvl1.org/DS1820_Temp_sensor)

### Parasite Power with two wires

![Parasite-Power-Schematic](http://i.stack.imgur.com/0MeGL.jpg)

[read more](http://electronics.stackexchange.com/questions/193300/digital-ic-that-draws-power-from-data-pins)

### Ancestors of this Lib:
- original pieces seem to be adopted from [OneWireSlave](http://robocraft.ru/blog/arduino/302.html)
- further development was done in [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2
- the current state of code has the concepts in common, but the codebase is a total rewrite