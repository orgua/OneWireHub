# HELP 
 
What to do if things don't work as expected?

- check if your arduino software up to date ([>v1.8.0](https://www.arduino.cc/en/software))
- update this lib to the latest release ([3.0.0](https://github.com/orgua/OneWireHub/releases))
- if you use an uncalibrated architecture the compilation-process will fail with an error, look at ./examples/debug/calibrate_by_bus_timing for an explanation
- check if clock-speed of the µC is set correctly (if possible) - test with simple blink example, 1sec ON should really need 1sec. timing is critical
- begin with a simple example like the ds18b20 (if possible). the ds18b20 doesn't support overdrive, so the OneWire-Host won't switch to higher data rates
- check if your setup is right: you need at least external power for your µC and a data line with ground line to your OneWire-Host (see section below)
- is there more than one OneWire-Host on the bus? It won't work!
- has any other sensor (real or emulated) ever worked with this OneWire-Host? -> the simplest device would be a ds2401
- if communication works, but is unstable please check with logic analyzer
    - maybe your OneWire-Host is slow and just needs a higher ONEWIREHUB_TIME_MSG_HIGH_TIMEOUT-value (see [OneWireHub_config.h line 37](https://github.com/orgua/OneWireHub/blob/main/src/OneWireHub_config.h#L37) )
- make sure that serial- and gpio-debugging is disabled (see [OneWireHub_config.h](https://github.com/orgua/OneWireHub/blob/main/src/OneWireHub_config.h)), especially when using overdrive (be aware! it may produce heisenbugs, timing is critical)
- on a slow arduino it can be helpful to disable the serial port completely to get reliable results -> at least comment out `serial.begin()`
- if you can provide a recording via logic-analyzer ([Logic 8](https://usd.saleae.com/products/saleae-logic-8) or similar) there should be chance we can help you
    - additional gpio-debug output can be enabled in [OneWireHub_config.h](https://github.com/orgua/OneWireHub/blob/main/src/OneWireHub_config.h): set USE_GPIO_DEBUG to 1 (it helps to track state changes of the hub)

```{Note}
If you checked all these points feel free to open an issue at [GitHub](https://github.com/orgua/OneWireHub) and describe your troubleshooting process.
Templates for a bug-report are provided - please provide that extra info and help the developer help you
```
