OneWireHub
==========

The OneWireHub is an Arduino compatible (and many more platforms) library to emulate OneWire-Slaves with support for various devices. The motivation is to offer a shared code base for all OneWire-Slaves. With a small overhead one µC can emulate up to 32 ICs simultaneously. 
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your home automation environment.

### Supported Slaves:
- **BAE910 (0xFC) multi purpose device (ADC, Clock, GPIO, PWM, EEPROM)**
- **DS1822 (0x22) Digital Thermometer, 12bit** -> use DS18B20 with different family code
- **DS18B20 (0x28) Digital Thermometer, 12bit** (also known as DS1820) 
- **DS18S20 (0x10) Digital Thermometer, 9bit** (also known as DS1920, use DS18B20 with different family code)
- **DS1990 (0x01) iButton** (DS2401 with same family code)
- **DS1990A (0x81) iButton** (DS2401 with different family code)
- **DS2401 (0x01) Serial Number**
- **DS2405 (0x05) Single address switch**
- **DS2408 (0x29) 8-Channel Addressable Switch**, GPIO Port-expander
- **DS2411 (0x01) Serial Number** -> use DS2401 with same family code
- **DS2413 (0x3A) Dual channel addressable switch with input-sensing**
- **DS2423 (0x1D) 4kbit RAM with Counter**
- **DS2431 (0x2D) 1kbit protected EEPROM** (also known as DS1972 or DS28E07, same FC)
- DS2432 (0x33) 1kbit protected EEPROM (basically a ds2431 with extra sha-engine)
- **DS2433 (0x23) 4Kbit EEPROM** (also known as DS1973)
- **DS2438 (0x26) Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit**
- **DS2450 (0x20) 4 channel A/D**
- **DS2501 (0x11, 0x91) 512bit EEPROM** -> use DS2502 with different family code
- **DS2502 (0x09, 0x89) 1kbit EEPROM, Add Only Memory** (also known as DS1982, same FC)
- **DS2503 (0x13) 4kbit EEPROM, Add Only Memory** (also known as DS1983, same FC) -> use DS2506 with different family code
- **DS2505 (0x0B) 16kbit EEPROM, Add Only Memory** (also known as DS1985, same FC) -> use DS2506 with different family code
- **DS2506 (0x0F) 64kbit EEPROM, Add Only Memory** (also known as DS1986, same FC)
- **DS2890 (0x2C) Single channel digital potentiometer - extended to 1-4 CH**
- Dell Power Supply (use DS2502 with family code set to 0x28)

Note: **Bold printed devices are feature-complete and were mostly tested with a DS9490 (look into the regarding example-file for more information) and a loxone system (when supported).**

### Features:
- supports up to 32 slaves (8 is standard setting), adjust HUB_SLAVE_LIMIT in src/OneWireHub_config.h to safe RAM & program space
- hot-plug: add and remove slaves as needed
- support for most onewire-features: MATCH ROM (0x55), SKIP ROM (0xCC), READ ROM (0x0F,0x33), RESUME COMMAND (0xA5)
   - **OVERDRIVE-Mode**: Master can issue OD SKIP ROM (0x13) or OD MATCH ROM (0x69) and slave stays in this mode till it sees a long reset -> OD-feature must be activated in config
   - ALARM SEARCH (0xEC) is NOT implemented yet!
- cleaner, faster code with c++11 features **(requires arduino sw 1.6.x or higher, >=1.6.10 recommended)**
   - use of constexpr instead of #define for better compiler-messages and cleaner code
   - use static-assertions for plausibility checks
   - user defined literals convert constants into needed format / unit
- hardware-dependencies are combined in "platform.h", synced with [Onewire-Lib](https://github.com/PaulStoffregen/OneWire)
   - supported: arduino zero, teensy, sam3x, pic32, [ATtiny](https://github.com/damellis/attiny), esp8266, nrf51822 (...)
   - tested architectures: atmega328 @ 16 MHz / arduino Uno, teensy3.2
   - for portability and tests the hub can be compiled on a PC with the supplied mock-up functions
   - at the moment the lib relies sole on loop-counting for timing, no direct access to interrupt or timers, **NOTE:** if you use an uncalibrated architecture the compilation-process will fail with an error, look at ./examples/debug/calibrate_by_bus_timing for an explanation
- Serial-Debug output can be enabled in src/OneWireHub_config.h: set USE_SERIAL_DEBUG to 1 (be aware! it may produce heisenbugs, timing is critical)
- GPIO-Debug output - shows status by issuing high-states (activate in src/OneWireHub_config.h, is a better alternative to serial debug)
   - during presence detection (after reset), 
   - after receiving / sending a whole byte (not during SEARCH ROM)
   - when duty()-subroutines of an attached slave get called 
   - during hub-startup it issues a 1ms long high-state (you can check the instruction-per-loop-value for your architecture with this)
- provide documentation, numerous examples, easy interface for hub and sensors

### Recent development (latest at the top):
- interface of hub and slave-devices has changed, check header-file or examples for more info
- rework / clean handling of timing-constants with user defined literals.
- extend const-correctness to all onewire-slaves and unify naming of functions across similar devices
- include tests into each device-example and add a lot of get()/set() for internal device-states
- full support for ds2423, ds2450 and ds2503/5/6
- fix and enhance ds2431, ds2433, ds2502, ds2890, probably every slave got a rework
- overdrive-support! must be enabled in config file - works with atmega328@16MHz
- rework send() and recv(), much more efficient -> less time without interupts (no missing time with millis())! AND code is more compact (ds2433.cpp shrinks from 176 to 90 LOC)
- rework Error-Handling-System (reduced a lot of overhead)
- no return value for hub.searchIDTree() or item.duty() needed anymore
- returns 1 if error occured in the following functions: recv(buf[]), send(), awaitTimeslot(), sendBit(), checkReset(), showPrescence(), recvAndProzessCmd()
- support for ds2408 (thanks to vytautassurvila) and ds2450
- offline calibration by watching the bus (examples/debug/calibrate_by_bus_timing)
   - branch for online calibration was abandoned because it took to much resources (DS18B20-Sketch compiled to 8434 // 482 bytes instead of 7026 // 426 bytes now) 
- cleaned up timing-fn (no guessing, no micros(), no delayMicroseconds())
- debug-pin shows state by issuing high-states - see explanation in "features"
- teensy3.2 tested: cleaned warnings, fixed port access, cleaned examples
- added or extended the ds2431, ds2431, ds2501, ds2502 (also tested)
- added ds2431 (thanks to j-langlois) and BAE910 (thanks to Giermann), Dell Power Supply (thanks to Kondi)
- prepare new timing-method which will replace the old one in the next couple of weeks (a 6µs millis() call at 8MHz is not suitable for OW) 
- support for skipROM-cmd if only one slave is present (thanks to Giermann)
- speed up atmel-crc-functions
- rework of error system, switch to enum, slaves can raise errors now & and Serial interferes less with OW-timings
- raise the maximal slave limit from 8 to 32, code adapts via variable dataTypes
- per-bit-CRC16 with sendAndCRC16() and sendAndCRC16() for load-balancing, 900ns/bit instead of 7µs/byte on Atmega328@16MHz
- faster CRC16 (ds2450 and ds2408 and ds2423), takes 5-7µs/byte instead of 10µs
- replace searchIDTree() algorithm, safes a lot of ram (debug-codeSize-4slaves.ino needs 3986 & 155 byte instead of 3928 & 891 byte) and allows >4 devices

### Plans for the future:
- implementation of ds2423
- alarm / conditional search
- irq-handled hub on supported ports, split lib into onewire() and onewireIRQ()
- test each example with real onewire-masters, for now it's tested with the onewire-lib and a loxone-system (ds18b20 passed)
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)
- [List of Maxim Sensors](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3989) (at the bottom)

### Connecting the HUB with the Network: 

![Onewire-Schematic](http://wiki.lvl1.org/images/1/15/Onewire.gif)

[read more](http://wiki.lvl1.org/DS1820_Temp_sensor)

### Parasite Power with two wires

![Parasite-Power-Schematic](http://i.stack.imgur.com/0MeGL.jpg)

**Note:** this will certainly not work with an emulated device. Powering a µController via GPIO is sometimes possible, but needs preparation and tests.

[read more](http://electronics.stackexchange.com/questions/193300/digital-ic-that-draws-power-from-data-pins)

### HELP - What to do if things don't work as expected?
- is your arduino software up to date (>v1.6.8)
- update this lib to the latest release (v2.0.0)
- if you use an uncalibrated architecture the compilation-process will fail with an error, look at ./examples/debug/calibrate_by_bus_timing for an explanation
- Serial-Debug output can be enabled in src/OneWireHub_config.h: set USE_SERIAL_DEBUG to 1 (be aware! it may produce heisenbugs, timing is critical)
- check if clock-speed of the µC is correctly set (if possible) - test with simple blinking example, 1sec ON should really need 1sec. timing is critical
- begin with a simple example like the ds18b20. the ds18b20 doesn't support overdrive, so the master won't switch to higher datarates
- check if your setup is right: you need at least external power for your µC and a dataline with groundline to your Onewire-Master
- is there more than one master on the bus? It won't work!
- has any other sensor ever worked with with master?
- is serial-debugging disabled (see src/OneWireHub_config.h)?
- if you can provide a recording via logic-analyzer (logic 8 or similar) there should be chance we can help you 
- if you checked all these points feel free to open an issue at [Github](https://github.com/orgua/OneWireHub)

### Ancestors of this Lib:
- original pieces seem to be adopted from [OneWireSlave](http://robocraft.ru/blog/arduino/302.html)
- further development was done in [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2
- the current code has just the concepts in common, but the codebase is a total rewrite