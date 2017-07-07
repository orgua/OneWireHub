/*
 *    Example-Code that emulates various Sensor - mostly for development
 *    --> attach sensors as needed
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire on the other side as Master
 *
 *    Compile size (program / ram)
 *      9232 & 706 byte with arduino 1.6.10 and onewirehub 2.0.1 for UNO
 *      9272 & 708 byte with arduino 1.8.03 and onewirehub 2.0.1 for UNO
 *      9288 & 708 byte with arduino 1.8.03 and onewirehub 2.0.2 for UNO
 */

#include "OneWireHub.h"

// include all libs to find errors
#include "BAE910.h"  //
#include "DS18B20.h" // Digital Thermometer
#include "DS2401.h"  // Serial Number
#include "DS2405.h"  // Single adress switch
#include "DS2408.h"  // 8-Channel Addressable Switch
#include "DS2413.h"  // Dual channel addressable switch
#include "DS2423.h"  // 4kb 1-Wire RAM with Counter
#include "DS2431.h"  //
#include "DS2433.h"  // 4Kb 1-Wire EEPROM
#include "DS2438.h"  // Smart Battery Monitor
#include "DS2450.h"  // 4 channel A/D
#include "DS2502.h"  //
#include "DS2890.h"  // Single channel digital potentiometer

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub      = OneWireHub(pin_onewire);
auto ds1822   = DS18B20(0x22, 0x0D, 0x01, 0x08, 0x02, 0x00, 0x00);
auto ds18B20  = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);      // Work - Digital Thermometer
auto ds18S20  = DS18B20(0x10, 0x0D, 0x01, 0x08, 0x0F, 0x02, 0x00);
auto ds2401a  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0A );    // Work - Serial Number
auto ds2401b  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0B );    // Work - Serial Number
auto ds2405   = DS2405( 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Single address switch
// auto ds2408   = DS2408( 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 8-Channel Addressable Switch
auto ds2413   = DS2413( 0x3A, 0x0D, 0x02, 0x04, 0x01, 0x03, 0x00 );    // Work - Dual channel addressable switch
// auto ds2423   = DS2423( 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
// auto ds2433   = DS2433( 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4Kb 1-Wire EEPROM
auto ds2438   = DS2438( 0x26, 0x0D, 0x02, 0x04, 0x03, 0x08, 0x00 );    //      - Smart Battery Monitor
auto ds2450   = DS2450( 0x20, 0x0D, 0x0A, 0x02, 0x04, 0x05, 0x00 );    //      - 4 channel A/D
auto ds2890A  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0A );    // Work - Single channel digital potentiometer
//auto ds2890B  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0B );
//auto ds2890C  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0C );

bool blinking()
{
    constexpr  uint32_t interval    = 500;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(pin_led, ledState);
        return 1;
    }
    return 0;
}

void setup()
{
    //Serial.begin(115200);
    Serial.println("OneWire-Hub Test with various Sensors");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    ds1822.setTemperature(21.0f);
    ds18S20.setTemperature(10.0f);
    hub.attach(ds1822);
    hub.attach(ds18B20);
    hub.attach(ds18S20);
    hub.attach(ds2401a);
    hub.attach(ds2405);
    //hub.attach(ds2401b);
    hub.attach(ds2413);
    hub.attach(ds2438);
    //hub.attach(ds2450);
    hub.attach(ds2890A);
    //hub.attach(ds2890B);
    //hub.attach(ds2890C);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.hasError()) hub.printError();

    // Blink triggers the state-change
    if (blinking())
    {
        // DS18B20
        static float temperature = 20.0;
        temperature += 0.1;
        if (temperature > 40.0) temperature = 10.0;
        ds18B20.setTemperature(temperature);
        //Serial.println(temperature);

        // DS2450
        static uint16_t p1, p2, p3, p4;
        p1 +=1;
        p2 +=2;
        p3 +=4;
        p4 +=8;
        ds2450.setPotentiometer(p1,p2,p3,p4);
    }
}