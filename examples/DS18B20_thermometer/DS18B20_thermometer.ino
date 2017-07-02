/*
 *    Example-Code that emulates a DS18B20
 *
 *    Tested with:
 *    - https://github.com/PaulStoffregen/OneWire --> DS18x20-Example, atmega328@16MHz as Slave
 *    - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
 */

#include "OneWireHub.h"
#include "DS18B20.h"  // Digital Thermometer, 12bit

constexpr uint8_t pin_led       { 13 };
constexpr uint8_t pin_onewire   { 8 };

auto hub    = OneWireHub(pin_onewire);

auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x00, 0xB2, 0x18, 0xDA, 0x00); // DS18B20: 9-12bit, -55 -  +85 degC
auto ds18s20 = DS18B20(0x10, 0x00, 0x00, 0xA2, 0x18, 0xDA, 0x00);                 // DS18S20: 9   bit, -55 -  +85 degC
auto ds1822  = DS18B20(0x22, 0x00, 0x00, 0x22, 0x18, 0xDA, 0x00);                 // DS1822:  9-12bit, -55 - +125 degC

bool blinking(void);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS18B20 Temperature-Sensor");
    Serial.flush();

    pinMode(pin_led, OUTPUT);

    // Setup OneWire
    hub.attach(ds18b20);
    hub.attach(ds18s20);
    hub.attach(ds1822);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.print("Test - set Temperatures to -56 degC (out of range): ");
    ds18b20.setTemperature(int8_t(-56));
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set Temperatures to -55 degC: ");
    ds18b20.setTemperature(int8_t(-55));
    ds18s20.setTemperature(int8_t(-55));
    Serial.print(ds18b20.getTemperature());
    Serial.print(", ");
    Serial.println(ds18s20.getTemperature());   // ds18s20 is limited to signed 9bit, so it could behave different

    Serial.print("Test - set Temperatures to 0 degC: ");
    ds18b20.setTemperature(int8_t(0));
    ds18s20.setTemperature(int8_t(0));
    Serial.print(ds18b20.getTemperature());
    Serial.print(", ");
    Serial.println(ds18s20.getTemperature());

    Serial.print("Test - set Temperatures to 21 degC: ");
    const int8_t temperature = 21;
    ds18b20.setTemperature(temperature);
    ds18s20.setTemperature(temperature);
    ds1822.setTemperature(temperature);
    Serial.print(ds18b20.getTemperature());
    Serial.print(", ");
    Serial.println(ds18s20.getTemperature());

    Serial.print("Test - set Temperatures to 85 degC: ");
    ds18b20.setTemperature(int8_t(85));
    ds18s20.setTemperature(int8_t(85));
    Serial.print(ds18b20.getTemperature());
    Serial.print(", ");
    Serial.println(ds18s20.getTemperature());

    Serial.print("Test - set Temperatures to 126 degC (out of range): ");
    ds1822.setTemperature(int8_t(126));
    Serial.println(ds1822.getTemperature());


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
        // Set temp
        static float temperature = 20.0;
        temperature += 0.1;
        if (temperature > 30.0) temperature = 20.0;
        ds18b20.setTemperature(temperature);
        ds18s20.setTemperature(temperature);
        ds1822.setTemperature(temperature);
        Serial.println(temperature);
    }
}

bool blinking(void)
{
    constexpr  uint32_t interval    = 1000;          // interval at which to blink (milliseconds)
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
