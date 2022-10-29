OneWireHub-TinyDs2434
==========

Special smaller version for ICs like the ds2434 (currently only this one). 

What changed? 

- no support for multidrop
- no support for basic onewire-commands (like search-rom, ...)
- no overdrive

### Implemented Slaves

- DS2434 - BatteryManagement used in some IBM Notebook-Batteries (similar to DS2436 (x1B), but without multidrop (and default ow-commands) and one less cmd)
