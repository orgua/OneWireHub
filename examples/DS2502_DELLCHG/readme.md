# DS2502_DELLCHG
This arduino sketch demonstrates how to emulate a Dell charger using OneWireHub.

Dell laptops check the authenticity of the charger upon plug-in, they will refuse to charge if they cannot determine the adapter capabilities (watts, voltage, current).
Each genuine Dell charger contains a DS2501, it has 2 data sections, the 8-byte lasered ROM and 128-byte add-only EEPROM. OneWireHub's implementation emulates both sections.
The lasered ROM starts with a 1-byte family code, a 6-byte serial number of the device, followed by a byte of CRC8 checksum of the first 7 bytes.
The DS2501 in a Dell charger has a family code of 0x28, which is different from the normal DS2502. It seems that the Dell laptop never reads the lasered ROM.
The EEPROM contains a 40-byte string followed a CRC16 check sum of the string. (42 bytes in total)
The string describes the capabilities and information about the charger, its format is tabulated as below:

| Offset | Length | Content                 | Description              |
|--------|--------|-------------------------|--------------------------|
|      0 |      4 | DELL                    | Manufacturer identifier  |
|      4 |      4 | 00AC                    | Adapter type             |
|      8 |      3 | 045                     | Watts (45W)              |
|     11 |      3 | 195                     | Tenths of a volt (19.5V) |
|     14 |      3 | 023                     | Tenths of amps (2.3A)    |
|     17 |     23 | CN0CDF577243865Q27F2A05 | Serial number            |
|     40 |      2 | 0x3D 0x94               | CRC-16/ARC (LSB first)   |

Source: https://github.com/KivApple/dell-charger-emulator
__The information given in the table came from reverse engineering and may not be entirely accurate.__

The string always seems to start with "DELL00AC", then followed by the __string representation__ of the power (3 bytes), voltage (3 bytes) and current (3 bytes).
The rest of the string is the serial number, which seems to be an arbitrary combination of letters and numbers.

The following C code demonstrates the calculation of the CRC16 of the 40-byte string:
```c
// This function assumes that the supplied string is exactly 40 bytes.
// crc_l and crc_h are the pointers to the lower and higher bytes of the calculated CRC16, respectively.
void crc(const char* string_40byte, uint8_t* crc_l, uint8_t* crc_h) {
  uint32_t crc = 0;
  for (int offset = 0; offset < 40; offset++) {
    uint8_t byte = string_40byte[offset];
    crc ^= byte;
    for (int i = 0; i<8; i++) {
      if (crc & 1)
        crc = (crc>>1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }
  *crc_l = crc & 0xFF;
  *crc_h = (crc >> 8) & 0xFF;
}
```

## Hardware configuration
The testing is performed with a ESP-01 module, which has a ESP8266 processor. GPIO2 of the module functions as the ID pin of the charger.
It should be aware that both GPIO0 and GPIO2 of the module must be pulled up to make ESP-01 execute the code.
The circuit requires an external 3.3V power source, as the parasite powering of the ID pin cannot provide enough power.
During the test, a combination of LM7805 and AMS1117 is used to convert the 19.5V output of a DC power supply to 3.3V, the temperature of the LM7805 reaches 60 celsius.
To achieve a better performance, it is recommended to use a switched DC-DC converter for supplying the 3.3V from 19.5V.
__Since the initialization of the ESP-01 module takes time, charging may not work properly if this circuit connects to the laptop before the DC power supply is turned on.__
The solution is to add a MOSFET (PMOS, e.g., IRF9540) as the bus switch and let the ESP-01 turn on the bus switch after it fully initializes the DS2501 emulator.

The prototype works with Dell Inspiron 15R N5110 and Dell Inspiron 15R 5521 at 130W.

__Do not make up a weak power adapter to a more powerful one. Overloading may permanently damage the converter.__

## More information on the string format:
1. https://github.com/garyStofer/DS2502_DELL_PS/blob/master/DS2502_DELL_PS.ino
2. https://nickschicht.wordpress.com/2009/07/15/dell-power-supply-fault/
3. https://github.com/KivApple/dell-charger-emulator
