/*
 *    Example-Code that emulates a DS2408 - an 8CH GPIO Port Extender
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire on the other side as Master
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 *    - BeagleBoneBlack running Linux 4.4.19. 1wire slave was accessed via original drivers:
 *
 *         #set all pins to 1 (xFF)
 *         echo -e '\xFF' |dd of=/sys/bus/w1/devices/29-010000000000/output bs=1 count=1
 *
 *         #get values for all pins
 *         dd if=/sys/bus/w1/devices/29-010000000000/state bs=1 count=1 | hexdump
 *
 */

#include "OneWireHub.h"
#include "DS2408.h"

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2408 = DS2408( DS2408::family_code, 0x00, 0x00, 0x08, 0x24, 0xDA, 0x00 );

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2408 - 8CH GPIO Port Extender");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2408);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test - clear Activity-State of GPIO 3");
    Serial.println(ds2408.getPinActivity(3));
    ds2408.setPinActivity(3,0);
    Serial.println(ds2408.getPinActivity(3));

    Serial.println("Test - clear State of GPIO 3");
    Serial.println(ds2408.getPinState(3));
    ds2408.setPinState(3,0);
    Serial.println(ds2408.getPinState(3));
    Serial.println(ds2408.getPinActivity(3)); // is active again

    // ds2408.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.hasError()) hub.printError();

    digitalWrite(10, ds2408.getPinState(0));
    digitalWrite(11, ds2408.getPinState(1));
    digitalWrite(2,  ds2408.getPinState(2));
    digitalWrite(3,  ds2408.getPinState(3));
    digitalWrite(4,  ds2408.getPinState(4));
    digitalWrite(5,  ds2408.getPinState(5));
    digitalWrite(6,  ds2408.getPinState(6));
    digitalWrite(7,  ds2408.getPinState(7));

    // Blink triggers the state-change
    if (blinking())
    {
        // this could be used to report up to eight states to 1wire master
        //ds2408.setPinState(0, digitalRead(10));
        Serial.print("0x");
        Serial.println(ds2408.getPinState(),BIN);
    }
}

bool blinking(void)
{
    const  uint32_t interval    = 1000;          // interval at which to blink (milliseconds)
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
