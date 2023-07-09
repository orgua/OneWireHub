# How the Hub works

- this layered description gives you a basic idea of how the functions inside the hub work together
- this will not tell you how the [onewire protocol](https://en.wikipedia.org/wiki/1-Wire) works - read a device datasheet or the link for that
- for further details try reading the header-files or check the examples

## Low Level - Hardware Access

- macros like `DIRECT_READ()` and `DIRECT_WRITE()` handle bus-access ([platform.h](https://github.com/orgua/OneWireHub/blob/main/src/platform.h))
- `checkReset()` and `showPresence()` are used to react to a beginning OneWire-Message
- `sendBit()`, `recvBit()` manage the information inside each timeslot issued by the OneWire-Host

## Mid Level - Onewire Protocol Logic

- `send()` and `recv()` can process data between device and OneWire-Host on byte-level (and do a CRC if needed)
- `recvAndProcessCmd()` handles basic commands of the OneWire-Host like: `search-rom`, `match-rom`, `skip-rom`, `read-rom`

## High Level - User Interaction

- `attach()` adds an instance of a device to the hub so the OneWire-Host can find it on the bus. there is a lot to do here. the device ID must be integrated into the tree-structure so that the hub knows how to react during a `search-rom`-command
- `detach()` takes the selected emulated device offline and restructures the search-tree
- `poll()` lets the hub listen to the bus. If there is a reset within a given time-frame it will continue to handle the message (show presence and receive commands), otherwise it will exit and you can do other stuff. the user should call this function as often as possible to intercept every message and therefore stay visible on the bus

## Device Level

- `device.duty()` gets automatically called when the OneWire-Host sends special commands (for example `match-rom`). now it is possible to handle device specific commands like "read memory" or "do temperature measurement". These commands deviate for each device.
- `device.setTemperature()` and `device.writeMemory()` for example are individual functions that handle core-functionality of the device and can be called by the user
