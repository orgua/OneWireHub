/*
 *    Example-Code that emulates a DS2438 Smart Battery Monitor
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 */

#include "OneWireHub.h"
#include "DS2438.h"  // Smart Battery Monitor

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);
auto ds2438 = DS2438( DS2438::family_code, 0x00, 0x00, 0x38, 0x24, 0xDA, 0x00 );    //      - Smart Battery Monitor

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2438 Smart Battery Monitor");

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds2438);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.print("Test: set Temperature in float 38 deg C: ");
    ds2438.setTemperature(38.0f);  // can vary from -55 to 125deg
    Serial.println(ds2438.getTemperature());

    Serial.print("Test: set Voltage to 8.70 V: ");
    ds2438.setVoltage(870); // 10mV-Steps
    Serial.println(ds2438.getVoltage());

    Serial.print("Test: set Current to 700 n: ");
    ds2438.setCurrent(700);  // hasn't any unit or scale
    Serial.println(ds2438.getCurrent());

    Serial.println("Test Write Text Data to page 3");
    constexpr char memory[] = "abcASCII";
    ds2438.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),3*8);

    Serial.println("Test Write binary Data to page 4&5");
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2438.writeMemory(mem_dummy, sizeof(mem_dummy), 4*8);

    Serial.print("Test Read binary Data to page 4: 0x");
    uint8_t mem_read[16];
    ds2438.readMemory(mem_read, 16, 4*8-1); // begin one byte earlier than page begins
    Serial.println(mem_read[2],HEX); // should read 0x11

    // values will be overwritten by the loop()

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

        ds2438.setTemperature(temp);
        ds2438.setVoltage(volt_10mV);
        ds2438.setCurrent(current);

        Serial.println(temp);
    }
}

bool blinking(void)
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
