# Recent Changes

v3.0.0

- improve documentation
- replace discriminatory language
- reduce name-collision
- improve config for hub (adjust device_limit & overdrive directly in main source-file)

v2.0.0

- travis CI and unittests
- more explicit coding, a lot of bugfixes with the help of unit tests (mainly esp8266, bea910, ds18b20)
- interface of hub and devices has changed, check header-file or examples for more info
- rework / clean handling of timing-constants with user defined literals.
- extend const-correctness to all onewire-devices and unify naming of functions across similar devices
- include tests into each device-example and add a lot of get()/set() for internal device-states
- full support for ds2423, ds2450 and ds2503/5/6
- fix and enhance ds2431, ds2433, ds2502, ds2890, probably every device got a rework / optimization
- overdrive-support! must be enabled in config file - works with atmega328@16MHz
- rework send() and recv(), much more efficient -> less time without interrupts (no missing time with millis())! AND code is more compact (ds2433.cpp shrinks from 176 to 90 LOC)
- rework Error-Handling-System (reduced a lot of overhead)
- no return value for hub.searchIDTree() or item.duty() needed anymore
- returns 1 if error occurred in the following functions: recv(buf[]), send(), awaitTimeslot(), sendBit(), checkReset(), showPresence(), recvAndProzessCmd()
- support for ds2408 (thanks to vytautassurvila) and ds2450
- offline calibration by watching the bus (examples/debug/calibrate_by_bus_timing)
    - branch for online calibration was abandoned because it took too many resources (DS18B20-Sketch compiled to 8434 // 482 bytes instead of 7026 // 426 bytes now)
- cleaned up timing-fn (no guessing, no micros(), no delayMicroseconds())
- debug-pin shows state by issuing high-states - see explanation in "features"
- teensy3.2 tested: cleaned warnings, fixed port access, cleaned examples
- added or extended the ds2431, ds2431, ds2501, ds2502 (also tested)
- added ds2431 (thanks to j-langlois) and BAE910 (thanks to Giermann), Dell Power Supply (thanks to Kondi)
- prepare new timing-method which will replace the old one in the next couple of weeks (a 6µs millis() call at 8MHz is not suitable for OW)
- support for skipROM-cmd if only one device is present (thanks to Giermann)
- speed up atmel-crc-functions
- rework of error system, switch to enum, peripheral devices can raise errors now & and Serial interferes less with OW-timings
- raise the maximal peripheral device limit from 8 to 32, code adapts via variable dataTypes
- per-bit-CRC16 with sendAndCRC16() and sendAndCRC16() for load-balancing, 900ns/bit instead of 7µs/byte on Atmega328@16MHz
- faster CRC16 (ds2450 and ds2408 and ds2423), takes 5-7µs/byte instead of 10µs
- replace searchIDTree() algorithm, safes a lot of ram (debug-codeSize-4devices.ino needs 3986 & 155 byte instead of 3928 & 891 byte) and allows >4 devices

## Roadmap - Plans for the future

- alarm / conditional search
- switch to delay() for fast enough controllers (instead of tick-counting)
- debug tool to determine timings of exotic OneWire-Hosts
- better interrupt-handling (save the state before disabling)
- irq-handled hub on supported ports, split lib into onewire() and onewireIRQ()
- test each example with real OneWire-Hosts, for now it's tested with the onewire-lib and a loxone-system (ds18b20 passed)
- [List of all Family-Codes](http://owfs.sourceforge.net/family.html)
- [List of Maxim Sensors](https://www.maximintegrated.com/en/app-notes/index.mvp/id/3989) (at the bottom)
- add examples for
    - more devices
    - overdrive mode
    - hw dependent code
