#include "OneWireHub.h"

// include all libs to find errors
#include "DS2401.h"  // Serial Number
#include "DS18B20.h" // Digital Thermometer
#include "DS2405.h"  // Single adress switch
#include "DS2408.h"  // 8-Channel Addressable Switch
#include "DS2413.h"  // Dual channel addressable switch
#include "DS2423.h"  // 4kb 1-Wire RAM with Counter
#include "DS2433.h"  // 4Kb 1-Wire EEPROM
#include "DS2438.h"  // Smart Battery Monitor
#include "DS2450.h"  // 4 channel A/D
#include "DS2890.h"  // Single channel digital potentiometer

const uint8_t led_PIN       = 13;         // the number of the LED pin
const uint8_t OneWire_PIN   = 8;

OneWireHub  hub = OneWireHub(OneWire_PIN);
auto ds18B20a = DS18B20(0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);      // Work - Digital Thermometer
auto ds18B20b = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);
auto ds2401   = DS2401( 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Serial Number
// auto ds2405   = DS2405( 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Single adress switch
// auto ds2408   = DS2408( 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 8-Channel Addressable Switch
auto ds2413   = DS2413( 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Dual channel addressable switch
// auto ds2423   = DS2423( 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
// auto ds2433   = DS2433( 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4Kb 1-Wire EEPROM
// auto ds2438   = DS2438( 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Smart Battery Monitor
auto ds2450   = DS2450( 0x20, 0x0D, 0x0A, 0x02, 0x04, 0x05, 0x00);      //      - 4 channel A/D
auto ds2890   = DS2890( 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Single channel digital potentiometer

bool blinking()
{
    const unsigned long interval = 250;           // interval at which to blink (milliseconds)
    static unsigned long nextMillis = millis(); // will store next time LED will updated
    static uint8_t ledState = LOW; // ledState used to set the LED

    unsigned long currentMillis = millis();

    if (currentMillis > nextMillis)
    {
        nextMillis = currentMillis + interval; // save the last time you blinked the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(led_PIN, ledState);
        return 1;
    }
    return 0;
}

void setup()
{
    // Debug
    Serial.begin(115200);
    Serial.println("OneWire-Hub Test with various Sensors");

    // put your setup code here, to run once:
    hub.attach(ds18B20a); // TODO: can there only be 4 sensors at once?
    hub.attach(ds18B20b);
    hub.attach(ds2401);
    hub.attach(ds2413);
    //hub.attach(ds2450); // TODO: still breaks the communication
    //hub.attach(ds2890);
    Serial.println(hub.calck_mask());
    Serial.println("config done");
}

void loop()
{
    // put your main code here, to run repeatedly:
    hub.waitForRequest(false);

    // Blink
    if (blinking())
    {
        // DS18B20
        static float temperature = 20.0;
        temperature += 0.1;
        if (temperature > 40.0) temperature = 10.0;
        //ds18B20a.setTemp(21);
        ds18B20b.setTemp(temperature);
        Serial.println(temperature);

        // DS2450
        static uint16_t p1, p2, p3, p4;
        p1 +=1;
        p2 +=2;
        p3 +=4;
        p4 +=8;
        ds2450.setPotentiometer(p1,p2,p3,p4);
    }
}