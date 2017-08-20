OneWireHub
==========

The OneWireHub is a sleek Arduino compatible (and many more platforms) library to emulate OneWire-Slaves with support for various devices. The motivation is to offer a shared code base for all OneWire-Slaves. With a small overhead one µC can emulate up to 32 ICs simultaneously. 
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your home automation environment.

[![Build Status](https://travis-ci.org/orgua/OneWireHub.svg?branch=master)](https://travis-ci.org/orgua/OneWireHub)

### Implemented Slaves:
- **BAE0910 (0xFC) multi purpose device (ADC, Clock, GPIO, PWM, EEPROM)**
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
- **DellAC (0x28) Dell Power Supply** (use DS2502 with different family code)

Note: **Bold printed devices are feature-complete and were mostly tested with a DS9490 (look into the regarding example-file for more information) and a loxone system (when supported).**

### Features:
- supports up to 32 slaves simultaneously (8 is standard setting), adjust HUB_SLAVE_LIMIT in src/OneWireHub_config.h to safe RAM & program space
   - implementation-overhead for the hub is minimal and even saves resources for >1 emulated device
- hot-plug: add and remove slaves as needed during operation
- support for most onewire-features: MATCH ROM (0x55), SKIP ROM (0xCC), READ ROM (0x0F,0x33), RESUME COMMAND (0xA5)
   - **OVERDRIVE-Mode**: Master can issue OD SKIP ROM (0x13) or OD MATCH ROM (0x69) and slave stays in this mode till it sees a long reset -> OD-feature must be activated in config
   - ALARM SEARCH (0xEC) is NOT implemented yet!
- cleaner, faster code with c++11 features **(requires arduino sw 1.6.x or higher, >=1.6.10 recommended)**
   - use of constexpr instead of #define for better compiler-messages and cleaner code
   - use static-assertions for plausibility checks
   - user defined literals convert constants into needed format / unit
- hardware-dependencies are combined in "platform.h", synced with [Onewire-Lib](https://github.com/PaulStoffregen/OneWire)
   - supported: arduino zero, teensy, pic32, [ATtiny](https://github.com/damellis/attiny), esp8266, esp32, raspberry (...)
   - tested architectures: atmega328 @ 16 MHz / arduino Uno, teensy3.2
   - for portability and tests the hub can be compiled on a PC with the supplied mock-up functions in platform.h
   - at the moment the lib relies sole on loop-counting for timing, no direct access to interrupt or timers, **NOTE:** if you use an uncalibrated architecture the compilation-process will fail with an error, look at ./examples/debug/calibrate_by_bus_timing for an explanation
- hub and slaves are unit tested and run for each supported architecture through travis CI
- Serial-Debug output can be enabled in src/OneWireHub_config.h: set USE_SERIAL_DEBUG to 1 (be aware! it may produce heisenbugs, timing is critical)
- GPIO-Debug output - shows status by issuing high-states (activate in src/OneWireHub_config.h, is a better alternative to serial debug)
   - during presence detection (after reset), 
   - after receiving / sending a whole byte (not during SEARCH ROM)
   - when duty()-subroutines of an attached slave get called 
   - during hub-startup it issues a 1ms long high-state (you can check the instruction-per-loop-value for your architecture with this)
- provide documentation, numerous examples, easy interface for hub and sensors

### Supported and tested Hardware
- embedded real life test
   - setup: run test-example, use ds9490-master, arduino 1.8.3, Windows 10 and the board-library named in the brackets
   - Arduino Uno ([Arduino AVR Boards](https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr))
   - Teensy 3.2 ([teensyduino](https://github.com/PaulStoffregen/cores))
   - Wemos D1 Mini ESP32S ([esp32](https://github.com/espressif/arduino-esp32))
   - Wemos Wifi & BT ESP32 ([esp32](https://github.com/espressif/arduino-esp32))
   - Wemos D1 R2 ([esp8266](https://github.com/esp8266/Arduino))
   - nodeMCU 1.0 ESP-12E ([esp8266](https://github.com/esp8266/Arduino))
   - ATtiny 84, 88 ([attiny](https://github.com/damellis/attiny))
- Travis CI (automated Continuous Integration) for different platforms
   - Arduino Uno ([Arduino AVR Boards](https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr))
   - Teensy 3.0, 3.1, 3.2, LC, 3.5, 3.6 ([teensyduino](https://github.com/PaulStoffregen/cores))
   - generic ESP8266 ([esp8266](https://github.com/esp8266/Arduino))
   - nodeMCU V2 ([esp8266](https://github.com/esp8266/Arduino))
   - espduino ([esp8266](https://github.com/esp8266/Arduino))
   - ESP32 dev module ([esp32](https://github.com/espressif/arduino-esp32))
   - Digispark tiny ([DigistumpArduino](https://github.com/digistump/DigistumpArduino))
- failing platforms
   - reason: current tick-counting implementation is not compatible with variable clock-speed
      - Arduino Due ([Arduino SAMD Boards (32-bits ARM Cortex-M3)](https://github.com/arduino/ArduinoCore-sam)) 
      - Arduino MKRZero ([Arduino SAMD Boards (32-bits ARM Cortex-M0+)](https://github.com/arduino/ArduinoCore-samd))
   - reason: gcc 4.8.3 is artificially limited to c++98  
      - Arduino Primo ([Arduino nRF52 Boards](https://github.com/arduino-org/arduino-core-nrf52)) 
      - RedBear [nRF51](https://github.com/RedBearLab/nRF51822-Arduino)
   - reason: value_ipl is unknown for this hardware
      - Arduino 101 ([Intel Curie Boards](https://github.com/01org/corelibs-arduino101))

### How does the Hub work
- this layered description gives you a basic idea of how the functions inside the hub work together
- this will not tell you how the [onewire protocol](https://en.wikipedia.org/wiki/1-Wire) works - read a device datasheet or the link for that
- Low Level - hardware access
   - macros like DIRECT_READ() and DIRECT_WRITE() handle bus-access (platform.h)
   - checkReset() and showPresence() are used to react to a beginning OW-Message
   - sendBit(), recvBit() manage the information inside each timeslot issued by the master
- Mid Level - onewire protocol logic
   - send() and recv() can process data between device and master on byte-level (and do a CRC if needed)
   - recvAndProcessCmd() handles basic commands of the master like: search-rom, match-rom, skip-rom, read-rom
- High Level - user interaction
   - attach() adds an instance of a ow-device to the hub so the master can find it on the bus. there is a lot to do here. the device ID must be integrated into the tree-structure so that the hub knows how to react during a search-rom-command  
   - detach() takes the selected emulated device offline and restructures the search-tree
   - poll() lets the hub listen to the bus. If there is a reset within a given time-frame it will continue to handle the message (show presence and receive commands), otherwise it will exit and you can do other stuff. the user should call this function as often as possible to intercept every message and therefore stay visible on the bus
- Slave Level:
   - slave.duty() gets automatically called when the master sends special commands (for example match-rom). now it is possible to handle device specific commands like "read memory" or "do temperature measurement". These commands deviate for each device.
   - slave.setTemperature() and slave.writeMemory() for example are individual functions that handle core-functionality of the device and can be called by the user
- for further details try reading the header-files or check the examples

### HELP - What to do if things don't work as expected?
- check if your arduino software up to date (>v1.8.0)
- update this lib to the latest release (v2.2.0)
- if you use an uncalibrated architecture the compilation-process will fail with an error, look at ./examples/debug/calibrate_by_bus_timing for an explanation
- check if clock-speed of the µC is set correctly (if possible) - test with simple blink example, 1sec ON should really need 1sec. timing is critical
- begin with a simple example like the ds18b20 (if possible). the ds18b20 doesn't support overdrive, so the master won't switch to higher data rates
- check if your setup is right: you need at least external power for your µC and a data line with ground line to your onewire-master (see section below)
- is there more than one master on the bus? It won't work!
- has any other sensor (real or emulated) ever worked with this master? -> the simplest device would be a ds2401
- if communication works, but is unstable please check with logic analyzer
   - maybe your master is slow and just needs a higher ONEWIRE_TIME_MSG_HIGH_TIMEOUT-value (see OneWireHub_config.h line 37)
- make sure that serial- and gpio-debugging is disabled (see src/OneWireHub_config.h), especially when using overdrive (be aware! it may produce heisenbugs, timing is critical)
- on a slow arduino it can be helpful to disable the serial port completely to get reliable results -> at least comment out serial.begin() 
- if you can provide a recording via logic-analyzer (logic 8 or similar) there should be chance we can help you 
   - additional gpio-debug output can be enabled in src/OneWireHub_config.h: set USE_GPIO_DEBUG to 1 (it helps tracking state changes of the hub)
- if you checked all these points feel free to open an issue at [Github](https://github.com/orgua/OneWireHub) and describe your troubleshooting process
   - please provide the following basic info: which µC and master do you use, software versions, what device do you try to emulate, what works, what doesn't

### Recent development (latest at the top):
- travis CI and unittests
- more explicit coding, a lot of bugfixes with the help of unit tests (mainly esp8266, bea910, ds18b20)
- interface of hub and slave-devices has changed, check header-file or examples for more info
- rework / clean handling of timing-constants with user defined literals.
- extend const-correctness to all onewire-slaves and unify naming of functions across similar devices
- include tests into each device-example and add a lot of get()/set() for internal device-states
- full support for ds2423, ds2450 and ds2503/5/6
- fix and enhance ds2431, ds2433, ds2502, ds2890, probably every slave got a rework / optimization
- overdrive-support! must be enabled in config file - works with atmega328@16MHz
- rework send() and recv(), much more efficient -> less time without interupts (no missing time with millis())! AND code is more compact (ds2433.cpp shrinks from 176 to 90 LOC)
- rework Error-Handling-System (reduced a lot of overhead)
- no return value for hub.searchIDTree() or item.duty() needed anymore
- returns 1 if error occured in the following functions: recv(buf[]), send(), awaitTimeslot(), sendBit(), checkReset(), showPresence(), recvAndProzessCmd()
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
- alarm / conditional search
- switch to delay() for fast enough controllers (instead of tick-counting)
- debug tool to determine timings of exotic masters
- better interrupt-handling (save the state before disabling)
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

### Ancestors of this Lib:
- original pieces seem to be adopted from [OneWireSlave](http://robocraft.ru/blog/arduino/302.html)
- further development was done in [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2
- the current code has just the concepts in common, but the codebase is a total rewrite