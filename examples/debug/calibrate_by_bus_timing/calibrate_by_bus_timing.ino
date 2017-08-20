/*
 *    Test-Code for calibration - this sketch determines the value for "instructions per loop" for your µC/CPU-architecture
 *
 *      setup: upload sketch to controller and hook it up to a OW-master, it will calibrate itself to the "seen" reset-pulses
 *      NOTE: you will need a serial-port to make this work
 *
 *      --> read value per serial-com and write it to /src/platform.h to YOUR specific architecture
 *          >>>>  constexpr uint8_t VALUE_IPL {0}; // instructions per loop
 *
 *      --> alternative: activate gpio-debug and measure 1ms-high-state after calibration, adapt VALUE_IPL accordingly to match 1ms
 *
 *      --> test it with a normal sensor-sketch (like ds18b20_thermometer.ini)
 *
 *      --> if it works please make a pullrequest or open an issue to report your determined value
 *          >>>> https://github.com/orgua/OneWireHub
 */

#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

constexpr uint8_t pin_led_dbg   { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub     = OneWireHub(pin_onewire); // do an bus-timing-calibration on first sensor-attachment

auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x02, 0x0B, 0x08, 0x01, 0x0D);    // Digital Thermometer

/////////////////////////////////////////////////////////////////////////
/////// Main Code              //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void setup()
{
    Serial.begin(115200);

    Serial.println("OneWire-Hub calibration by observing the OW-Bus");
    Serial.flush();

    hub.attach(ds18b20);

    pinMode(pin_led_dbg, OUTPUT);
    digitalWrite(pin_led_dbg,HIGH);
}

void loop()
{
    digitalWrite(pin_led_dbg,HIGH);
    const timeOW_t value_ipl = hub.waitLoopsCalibrate();
    digitalWrite(pin_led_dbg,LOW);

    Serial.print(value_ipl);
    Serial.println("\t instructions per loop");

    hub.waitLoopsDebug();

    hub.poll();


    // advanced calibration loop --> try to track and measure it with a logic analyzer
    if (false)
    {
        io_reg_t debug_bitMask = PIN_TO_BITMASK(pin_led_dbg);
        volatile io_reg_t *debug_baseReg = PIN_TO_BASEREG(pin_led_dbg);
        pinMode(pin_led_dbg, OUTPUT);
        DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);

        const timeOW_t loops_1ms = timeUsToLoops(uint16_t(VALUE1k));
        timeOW_t loops_left = 1;
        while (loops_left)
        {
            DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask); // Fast high low flank
            DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);

            uint32_t retries = loops_1ms; // volatile needs ~2.5ms for the 1ms-loop .... but in a separate function it works as expected
            while ((!DIRECT_READ(debug_baseReg, debug_bitMask)) && (--retries)); // try to wait 1ms while LOW

            DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask); // switch to HIGH

            retries += loops_1ms;
            while ((DIRECT_READ(debug_baseReg, debug_bitMask)) && (--retries)); // try to wait 1ms while HIGH
            loops_left = retries;

            DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
        }

        loops_left = 1;
        while (loops_left)
        {
            DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask); // switch to HIGH

            uint32_t retries = loops_1ms;
            while ((DIRECT_READ(debug_baseReg, debug_bitMask)) && (--retries)); // try to wait 1ms while HIGH

            DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);

            retries += loops_1ms;
            while (!(DIRECT_READ(debug_baseReg, debug_bitMask)) && (--retries)); // try to wait 1ms while LOW
            loops_left = retries;

            DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask); // Fast high low flank
            DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
        }
    }
}



