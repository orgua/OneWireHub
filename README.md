OneWireHub
==========

### An Arduino compatible Tool to emulate OneWire-Slaves with support for various simultaneously devices:
- **DS2401 Serial Number** (working)
- **DS18B20 Digital Thermometer, 12bit** (working)
- DS2405 Single adress switch
- DS2408 8-Channel Addressable Switch
- **DS2413 Dual channel addressable switch** (working)
- DS2423 4kb 1-Wire RAM with Counter
- DS2433 4Kb 1-Wire EEPROM
- DS2438 Smart Battery Monitor, measures temperature, 2x voltage and current (10bit)
- DS2450 4 channel A/D
- **DS2890 Single channel digital potentiometer** (working)

### Possible extensions in the future:
- working DS2438
- DS1822 0x22 Thermometer, 12bit
- DS1963S 0x18 iButton
- DS1990 0x01 iButton
- MAX31850 0x3B thermocouple-to-digital converter 14bit
- ~~DS18S20 0x10 Thermometer 9bit~~
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)

### History of this Lib:
- original pieces seem to be adopted from [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from @MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire) 
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by @Shagrat2

### Connecting the HUB with the Network: 

![Onewire-Schematic](http://wiki.lvl1.org/images/1/15/Onewire.gif)

[read more](http://wiki.lvl1.org/DS1820_Temp_sensor)

### Parasite Power with two wires

![Parasite-Power-Schematic](http://i.stack.imgur.com/0MeGL.jpg)

[read more](http://electronics.stackexchange.com/questions/193300/digital-ic-that-draws-power-from-data-pins)
