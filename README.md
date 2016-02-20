OneWireHub
==========

### An Arduino compatible Tool to emulate OneWire-Slaves with support for various simultaneously devices:
- **DS1820 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x22)
- **DS18B20 Digital Thermometer, 12bit** (working)
- **DS18S20 Digital Thermometer, 12bit** (use DS18B20 with family code set to 0x10)
- **DS2401 Serial Number** (working)
- DS2405 Single adress switch
- DS2408 8-Channel Addressable Switch, GPIO Portexpander
- **DS2413 Dual channel addressable switch** (working)
- DS2423 4kb 1-Wire RAM with Counter
- DS2433 4Kb 1-Wire EEPROM
- **DS2438 Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit** (working)
- DS2450 4 channel A/D
- **DS2890 Single channel digital potentiometer** (working)

### Recent development (newest on top): 
- implement and test ds2438
- fix bug: only one ds2401 possible?
- fix bug: buffer-overrun when using more than 4 slaves 
- replace search() algorithm, safes a lot of ram (HubTest-Minimal4 needs 3986 & 155 byte instead of 3928 & 891 byte)
- fix bug: infinite loop when waitForRequest() is called without attached sensor
- fix bug: infinite loop when (for example) >=1 ds2401 are attached and waitForRequest() is called
- fix bug: temp-calculation was wrong (ds18b20, ds2438) and used something like round(abs(floor(float-value)))
- better documentation, more examples, easier interface for hub
- cleaner, faster code
- make OneWireHub compatible to arduino library-manager

### Possible extensions in the future:
- DS1963S 0x18 iButton
- DS1990 0x01 iButton
- MAX31850 0x3B thermocouple-to-digital converter 14bit
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)

### History of this Lib:
- original pieces seem to be adopted from [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2

### Connecting the HUB with the Network: 

![Onewire-Schematic](http://wiki.lvl1.org/images/1/15/Onewire.gif)

[read more](http://wiki.lvl1.org/DS1820_Temp_sensor)

### Parasite Power with two wires

![Parasite-Power-Schematic](http://i.stack.imgur.com/0MeGL.jpg)

[read more](http://electronics.stackexchange.com/questions/193300/digital-ic-that-draws-power-from-data-pins)
