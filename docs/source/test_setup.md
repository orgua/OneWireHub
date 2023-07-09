# Supported & tested Hardware

## Embedded real life Tests

### Setup 

- manual process of the developer
- used software: arduino 1.8.3 or newer, Windows 10 and the board-library named in the brackets (below)
- flash [test-example](https://github.com/orgua/OneWireHub/blob/main/examples/OneWireHubTest/OneWireHubTest.ino), 
- use DS9490 as OneWire-Host

### Tested MCU-Boards

- Arduino Uno ([Arduino AVR Boards](https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr))
- Teensy 3.2 ([teensyduino](https://github.com/PaulStoffregen/cores))
- Wemos D1 Mini ESP32S ([esp32](https://github.com/espressif/arduino-esp32))
- Wemos Wifi & BT ESP32 ([esp32](https://github.com/espressif/arduino-esp32))
- Wemos D1 R2 ([esp8266](https://github.com/esp8266/Arduino))
- nodeMCU 1.0 ESP-12E ([esp8266](https://github.com/esp8266/Arduino))
- ATtiny 84, 88 ([attiny](https://github.com/damellis/attiny))

## Automated Continuous Integration 

[![CompileTests](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml/badge.svg)](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml)

### Setup

Platform-IO is used to compile the examples for different target-architectures. For details see the Action-Script (Badge above).

### Tested MCU-Architectures

- Arduino Uno ([Arduino AVR Boards](https://github.com/arduino/Arduino/tree/master/hardware/arduino/avr))
- Teensy 3.0, 3.1, 3.2, LC, 3.5, 3.6 ([teensyduino](https://github.com/PaulStoffregen/cores))
- generic ESP8266 ([esp8266](https://github.com/esp8266/Arduino))
- nodeMCU V2 ([esp8266](https://github.com/esp8266/Arduino))
- espduino ([esp8266](https://github.com/esp8266/Arduino))
- ESP32 dev module ([esp32](https://github.com/espressif/arduino-esp32))
- Digispark tiny ([DigistumpArduino](https://github.com/digistump/DigistumpArduino))

## Failing Platforms

TODO: this will change with V3 of the hub

- reason: current tick-counting implementation is not compatible with variable clock-speed
    - Arduino Due ([Arduino SAMD Boards (32-bits ARM Cortex-M3)](https://github.com/arduino/ArduinoCore-sam))
    - Arduino MKRZero ([Arduino SAMD Boards (32-bits ARM Cortex-M0+)](https://github.com/arduino/ArduinoCore-samd))
- reason: gcc 4.8.3 is artificially limited to c++98
    - Arduino Primo ([Arduino nRF52 Boards](https://github.com/arduino-org/arduino-core-nrf52))
    - RedBear [nRF51](https://github.com/RedBearLab/nRF51822-Arduino)
- reason: value_ipl is unknown for this hardware
    - Arduino 101 ([Intel Curie Boards](https://github.com/01org/corelibs-arduino101))
