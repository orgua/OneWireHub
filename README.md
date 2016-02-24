OneWireHub
==========

The OneWireHub is an Arduino and Raspberry Pi (WiringPI) compatible library to emulate OneWire-Slaves with support for various devices. The motivation is to offer a shared code base for all OneWire-Slaves. With a small overhead one µC can emulate up to 32 ICs simultaneously. 
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your environment.

### Supported Slaves (bold ones are feature-complete):
- **DS1822 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x22)
- **DS18B20 Digital Thermometer, 12bit** 
- **DS18S20 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x10)
- **DS1990 iButton** (use DS2401 with same family code 0x01)
- **DS2401 Serial Number**
- **DS2405 Single address switch**
- DS2408 8-Channel Addressable Switch, GPIO Port-expander
- **DS2413 Dual channel addressable switch with input-sensing**
- DS2423 4kb 1-Wire RAM with Counter
- DS2433 4Kb 1-Wire EEPROM
- **DS2438 Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit**
- DS2450 4 channel A/D
- **DS2890 Single channel digital potentiometer - extended to 1-4 CH**

### Further features:
- hot-plug slaves as needed
- supports up to 32 slaves, set ONEWIRESLAVE_LIMIT in OneWireHub.h to safe some RAM (8 is standard)
- cleaner, faster code with c++11 features (requires arduino 1.6.x or higher)
- arduino-dependencies are found in the mockup "arduino.h" (for portability and tests)
- hardware-dependencies are found in "platform.h", synced with onewire-lib
- good documentation, numerous examples, easy interface for hub and sensors

### Recent development (latest at the top): 
- raise the maximal slave limit to 32 (set the ONEWIRESLAVE_LIMIT-parameter in OneWireHub.h), takes only ~100b extra program-space
- open up for a lot more platforms with "platform.h" (taken from onewire-lib)
- fix bug: open-drain violation on slave side
- per-bit-CRC16 with sendAndCRC16() and sendAndCRC16() for load-balancing, 900ns/bit instead of 7µs/byte on Atmega328@16MHz
- add examples for onewire-master, for testing the bus
- rework of checkReset() and showPresence() - Hub is much more reliable now and it saves ~120 byte program-space
- faster CRC16 (ds2450 and ds2408 and ds2423), takes 5-7µs/byte instead of 10µs
- refactored the interface: hub.poll() replaces hub.waitForRequest()
- extended ds2890 to up to 4CH (datasheet has it covered), ds2413, ds2413 --> feature-complete
- implement and test ds2438
- fix bug: only one ds2401 possible?
- fix bug: buffer-overrun when using more than 4 active slaves 
- replace search() algorithm, safes a lot of ram (debug-codeSize-4slaves.ino needs 3986 & 155 byte instead of 3928 & 891 byte)
- fix bug: infinite loop when waitForRequest() is called without attached sensor
- fix bug: infinite loop when (for example) >=1 ds2401 are attached and waitForRequest() is called
- fix bug: temp-calculation was wrong (ds18b20, ds2438) and used something like round(abs(floor(float-value)))
- make OneWireHub compatible to arduino library-manager

### Plans for the future:
- implementation of ds2450
- rework the onewire-timings
- add table of tested sensors 
- irq-handled hub on supported ports, split lib into onewire() and onewireIRQ()
- work on the TODOs in the code
- test each example with real onewire-masters, for now it's tested with the onewire-lib and a loxone-system (ds18b20 passed)
- bug: infinite loop in waitForRequest() if no sensor is read out (scratchpad or sim)
- add MAX31850 0x3B thermocouple-to-digital converter 14bit
- ~~DS1963S 0x18 iButton, datasheet under NDA~~
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)

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
- the current state of code has the concepts in common, but the codebase is a (nearly) total rewrite