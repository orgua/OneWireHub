OneWireHub-DS2434-tiny
======================

Special smaller version for ICs like the ds2434 (currently only this one).

### Advantages

- much simpler program flow
- less program storage, making it fit on smaller attiny
  - test for 701c-Example on Arduino Uno
  - 4318 byte firmware & 392 byte Ram for orginal Hub-Source
  - 2738 byte firmware & 294 byte Ram for this tiny lib

### What changed?

- removed support for multidrop
- removed support for basic onewire-commands (like search-rom, ...)
- removed overdrive
- removed debug-code

### Implemented Slaves

- DS2434 - BatteryManagement used in some IBM Notebook-Batteries (similar to DS2436 (x1B), but without multidrop (and default ow-commands) and one less cmd)
