# OneWireHub

The OneWireHub is a sleek Arduino compatible (and many more platforms) library to emulate OneWire-Periphery with support for various devices & sensors. The motivation is to offer a shared code base for all OneWire-Periphery-Devices. With a small overhead one µC can emulate up to 32 ICs simultaneously.
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your home automation environment.

[![CompileTests](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml/badge.svg)](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml)
[![Documentation](https://github.com/orgua/OneWireHub/actions/workflows/sphinx_to_pages.yml/badge.svg)](https://orgua.github.io/OneWireHub/)

## Links

- [Documentation](https://orgua.github.io/OneWireHub/)
- [Main-Repository](https://github.com/orgua/OneWireHub)
- [Issue-Tracker](https://github.com/orgua/OneWireHub/issues)
- [Releases](https://github.com/orgua/OneWireHub/releases)

## Ancestors of this Lib

- original pieces seem to be adopted from [OneWireSlave](http://robocraft.ru/blog/arduino/302.html)
- further development was done in [OneWireSlave](https://github.com/MarkusLange/OneWireSlave) from MarkusLange and [OneWire](https://github.com/PaulStoffregen/OneWire)
- first implementation of the [OneWireHub](https://github.com/Shagrat2/OneWireHub) by Shagrat2
- the current code has just the concepts in common, but the codebase is a total rewrite
