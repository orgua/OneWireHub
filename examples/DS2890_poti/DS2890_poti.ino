/*
 *    Example-Code that emulates a DS2890 Single channel digital potentiometer (datasheet has it covered for up to 4 CH)
 *
 *    Tested with
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2890.h"  // Single channel digital potentiometer

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2890 = DS2890( 0x2C, 0x00, 0x00, 0x90, 0x28, 0xDA, 0x00 );    // Work - Single channel digital potentiometer

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2890 Single channel digital potentiometer");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2890);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Potentiometer 0");
    Serial.println(ds2890.getPotentiometer(0));
    ds2890.setPotentiometer(0,240);
    Serial.println(ds2890.getPotentiometer(0));

    Serial.println("Test Potentiometer 1");
    Serial.println(ds2890.getPotentiometer(1));
    ds2890.setPotentiometer(1,255);
    Serial.println(ds2890.getPotentiometer(1));

    Serial.println("Test Read Control Register");
    Serial.println(ds2890.getRegCtrl());

    Serial.println("Test Read Feature Register");
    Serial.println(ds2890.getRegFeat());

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();

    // Blink triggers the state-change
    if (blinking())
    {
        Serial.print("Potentiometer: ");
        Serial.print(ds2890.getPotentiometer(0));
        Serial.print(" ");
        Serial.print(ds2890.getPotentiometer(1));
        Serial.print(" ");
        Serial.print(ds2890.getPotentiometer(2));
        Serial.print(" ");
        Serial.print(ds2890.getPotentiometer(3));
        Serial.print(" of 255 with config: ");
        Serial.println(ds2890.getRegCtrl());

    }
}

bool blinking(void)
{
    const  uint32_t interval    = 500;          // interval at which to blink (milliseconds)
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
