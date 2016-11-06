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

auto ds18b20 = DS18B20(DS18B20::family_code, 0x00, 0x00, 0xB2, 0x18, 0xDA, 0x00);    // Digital Thermometer
auto ds18s20 = DS18B20(0x10, 0x00, 0x00, 0xA2, 0x18, 0xDA, 0x00);    // Digital Thermometer
auto ds1822  = DS18B20(0x22, 0x00, 0x00, 0x22, 0x18, 0xDA, 0x00);    // Digital Thermometer

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
    Serial.print("Test - set int16-Temperatures to -56 degC (out of range): ");
    ds18b20.setTemperature(int8_t(-56)); // [-55;+85] degC
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set int16-Temperatures to -55 degC: ");
    ds18b20.setTemperature(int8_t(-55)); // [-55;+85] degC
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set int16-Temperatures to 0 degC: ");
    ds18b20.setTemperature(int8_t(0)); // [-55;+85] degC
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set int16-Temperatures to 21 degC: ");
    const int8_t temperature = 21;
    ds18b20.setTemperature(temperature); // [-55;+85] degC
    ds18s20.setTemperature(temperature);
    ds1822.setTemperature(temperature);
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set int16-Temperatures to 85 degC: ");
    ds18b20.setTemperature(int8_t(85)); // [-55;+85] degC
    Serial.println(ds18b20.getTemperature());

    Serial.print("Test - set int16-Temperatures to 86 degC (out of range): ");
    ds18b20.setTemperature(int8_t(86)); // [-55;+85] degC
    Serial.println(ds18b20.getTemperature());


    Serial.println("config done");
};

void loop()
{
    // following function must be called periodically
    hub.poll();
    // this part is just for debugging (USE_SERIAL_DEBUG in OneWire.h must be enabled for output)
    if (hub.getError()) hub.printError();

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
