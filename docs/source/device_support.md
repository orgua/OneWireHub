# Implemented Devices

The following list gives a compact overview for implemented peripheral devices that can be emulated by the OneWireHub-Library. The device-name is followed by the family code (in brackets) and a short description of the functionality.

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
- **DS2430A (0x14) 256bit EEPROM & 64bit OTP** (also known as DS1971)
- **DS2431 (0x2D) 1kbit protected EEPROM** (also known as DS1972 or DS28E07, same FC)
- DS2432 (0x33) 1kbit protected EEPROM (basically a ds2431 with extra sha-engine)
- **DS2433 (0x23) 4kbit EEPROM** (also known as DS1973)
- DS2434 BatteryManagement used in some IBM Notebook-Batteries -> moved to [special branch of the Hub](https://github.com/orgua/OneWireHub/tree/tiny-ds2434)
- **DS2438 (0x26) Smart Battery Monitor, measures temperature, 2x voltage and current, 10bit**
- **DS2450 (0x20) 4 channel A/D**
- **DS2501 (0x11, 0x91) 512bit EEPROM** -> use DS2502 with different family code
- **DS2502 (0x09, 0x89) 1kbit EEPROM, Add Only Memory** (also known as DS1982, same FC)
- **DS2503 (0x13) 4kbit EEPROM, Add Only Memory** (also known as DS1983, same FC) -> use DS2506 with different family code
- **DS2505 (0x0B) 16kbit EEPROM, Add Only Memory** (also known as DS1985, same FC) -> use DS2506 with different family code
- **DS2506 (0x0F) 64kbit EEPROM, Add Only Memory** (also known as DS1986, same FC)
- **DS2890 (0x2C) Single channel digital potentiometer - extended to 1-4 CH**
- **DellAC (0x28) Dell Power Supply** (use DS2502 with different family code)

```{Note}
**Bold** printed devices are feature-complete and were mostly tested with a DS9490 (look into the regarding example-file for more information) and a loxone system (when supported).
```