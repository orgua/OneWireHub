OneWireHub
==========

An Arduino compatible Tool to emulate OneWire-Slaves with support for various simultaneously devices.

### Support for (bold ones are feature-complete):
- **DS1822 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x22)
- **DS18B20 Digital Thermometer, 12bit** 
- **DS18S20 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x10)
- **DS1990 iButton** (use DS2401 with same family code 0x01)
- **DS2401 Serial Number**
- **DS2405 Single address switch**
- DS2408 8-Channel Addressable Switch, GPIO Portexpander
- **DS2413 Dual channel addressable switch with input-sensing**
- DS2423 4kb 1-Wire RAM with Counter
- DS2433 4Kb 1-Wire EEPROM
- **DS2438 Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit**
- DS2450 4 channel A/D
- **DS2890 Single channel digital potentiometer - extended to 1-4 CH**

### Recent development (latest at the top): 
- extended ds2890 to up to 4CH (datasheet has it covered), ds2413, ds2413 --> feature-complete
- implement and test ds2438
- fix bug: only one ds2401 possible?
- fix bug: buffer-overrun when using more than 4 active slaves 
- replace search() algorithm, safes a lot of ram (HubTest-Minimal4 needs 3986 & 155 byte instead of 3928 & 891 byte)
- fix bug: infinite loop when waitForRequest() is called without attached sensor
- fix bug: infinite loop when (for example) >=1 ds2401 are attached and waitForRequest() is called
- fix bug: temp-calculation was wrong (ds18b20, ds2438) and used something like round(abs(floor(float-value)))
- better documentation, more examples, easier interface for hub
- cleaner, faster code with c++11 features (requires arduino 1.6.6 or higher)
- make OneWireHub compatible to arduino library-manager

### Possible extensions in the future:
- implementation of ds2450
- rework of the onewire-timings
- refactoring the interface
- work on the TODOs in the code
- bug: infinite loop in waitForRequest() if no sensor is read out (scratchpad or sim)
- MAX31850 0x3B thermocouple-to-digital converter 14bit
- ~~DS1963S 0x18 iButton, datasheet under NDA~~
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)

___

### Connecting the HUB with the Network: 

![Onewire-Schematic](http://wiki.lvl1.org/images/1/15/Onewire.gif)

[read more](http://wiki.lvl1.org/DS1820_Temp_sensor)

### Parasite Power with two wires

![Parasite-Power-Schematic](http://i.stack.imgur.com/0MeGL.jpg)

[read more](http://electronics.stackexchange.com/questions/193300/digital-ic-that-draws-power-from-data-pins)

___

### Ancestors of this Lib:
- original pieces seem to be adopted from [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2