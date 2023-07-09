# Introduction

The OneWireHub is a sleek Arduino compatible (and many more platforms) library to emulate OneWire-Periphery with support for various devices & sensors. The motivation is to offer a shared code base for all OneWire-Periphery-Devices. With a small overhead one µC can emulate up to 32 ICs simultaneously.
The main goal is to use modern sensors (mainly [I2C](https://github.com/orgua/iLib) or SPI interface) and transfer their measurements into one or more emulated ds2438 which have 4x16bit registers for values. This feature removes the limitations of modern house-automation-systems. Add humidity, light and other sensors easy to your home automation environment.

[![CompileTests](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml/badge.svg)](https://github.com/orgua/OneWireHub/actions/workflows/compile.yml)
[![Documentation](https://github.com/orgua/OneWireHub/actions/workflows/sphinx_to_pages.yml/badge.svg)](https://orgua.github.io/OneWireHub/)

## Links

- [Documentation](https://orgua.github.io/OneWireHub/)
- [Main-Repository](https://github.com/orgua/OneWireHub)
- [Issue-Tracker](https://github.com/orgua/OneWireHub/issues)
- [Releases](https://github.com/orgua/OneWireHub/releases)