/*
 *    Example-Code that emulates a DS2438 Smart Battery Monitor
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire
 *    - DS9490
 */

#include "OneWireHub.h"
#include "DS2438.h"  // Smart Battery Monitor

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2438 = DS2438( DS2438::family_code, 0x00, 0x00, 0x38, 0x24, 0xDA, 0x00 );    //      - Smart Battery Monitor

bool blinking()
{
    const  uint32_t interval    = 2000;          // interval at which to blink (milliseconds)
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
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2438 Smart Battery Monitor");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2438);

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        static float    temp      = 10.0;
        static uint16_t volt_10mV = 10;
        static uint16_t current   = 10;

        if ((temp += 0.05) > 30.0) temp = 10.0;
        if ((volt_10mV++) > 200 ) volt_10mV = 10;
        if ((current++)   > 200 ) current = 10;

        ds2438.setTemp(temp);
        ds2438.setVolt(volt_10mV);
        ds2438.setCurr(current);

        Serial.println(temp);
    };
};

/*
 *   A1.6.7 compiles to 7592 // 374 bytes
 *   A1.6.8 compiles to 5626 // 310 bytes @ v0.9.1
 */