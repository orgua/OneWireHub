//
//
//
#include <iostream>

using namespace std;

#include "src/OneWireHub.h"

// include all libs to find errors
#include "src/BAE910.h"
#include "src/DS18B20.h" // Digital Thermometer
#include "src/DS2401.h"  // Serial Number
#include "src/DS2405.h"  // Single adress switch
#include "src/DS2408.h"  // 8-Channel Addressable Switch
#include "src/DS2413.h"  // Dual channel addressable switch
#include "src/DS2423.h"  // 4kb 1-Wire RAM with Counter
#include "src/DS2431.h"  // 1kb 1-Wire EEPROM
#include "src/DS2433.h"  // 4Kb 1-Wire EEPROM
#include "src/DS2438.h"  // Smart Battery Monitor
#include "src/DS2450.h"  // 4 channel A/D
#include "src/DS2502.h"  // 1kb EEPROM
#include "src/DS2506.h"  // 64kb EEPROM
#include "src/DS2890.h"  // Single channel digital potentiometer



// taken from OneWireHubTest.ino

const uint8_t OneWire_PIN   = 8;

auto hubA      = OneWireHub(OneWire_PIN);
auto hubB      = OneWireHub(OneWire_PIN);
auto hubC      = OneWireHub(OneWire_PIN);

auto ds1822   = DS18B20(0x22, 0x0D, 0x01, 0x08, 0x02, 0x00, 0x00);
auto ds18B20  = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);      // Work - Digital Thermometer
auto ds18S20  = DS18B20(0x10, 0x0D, 0x01, 0x08, 0x0F, 0x02, 0x00);
auto ds2401a  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0A );    // Work - Serial Number
auto ds2401b  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0B );    // Work - Serial Number
auto ds2405   = DS2405( 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Single address switch
auto ds2408   = DS2408( 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 8-Channel Addressable Switch
auto ds2413   = DS2413( 0x3A, 0x0D, 0x02, 0x04, 0x01, 0x03, 0x00 );    // Work - Dual channel addressable switch

auto ds2423   = DS2423( 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
auto ds2431   = DS2431( 0x2D, 0xE8, 0x9F, 0x90, 0x0E, 0x00, 0x00 );    // Work - 1kb 1-Wire EEPROM
auto ds2433   = DS2433( 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4Kb 1-Wire EEPROM
auto ds2438   = DS2438( 0x26, 0x0D, 0x02, 0x04, 0x03, 0x08, 0x00 );    //      - Smart Battery Monitor
auto ds2450   = DS2450( DS2450::family_code, 0x00, 0x00, 0x50, 0x24, 0xDA, 0x00 ); //      - 4 channel A/D
auto ds2502   = DS2502( DS2502::family_code, 0x00, 0xA0, 0x02, 0x25, 0xDA, 0x00 );
auto ds2501a  = DS2502( 0x91, 0x00, 0xA0, 0x01, 0x25, 0xDA, 0x00 );
auto ds2501b  = DS2502( 0x11, 0x00, 0xB0, 0x02, 0x25, 0xDA, 0x00 );

auto ds2503   = DS2506( 0x13, 0x00, 0x00, 0x03, 0x25, 0xDA, 0x00 );
auto ds2505   = DS2506( 0x0B, 0x00, 0x00, 0x05, 0x25, 0xDA, 0x00 );
auto ds2506   = DS2506( 0x0F, 0x00, 0x00, 0x06, 0x25, 0xDA, 0x00 );
auto ds2890A  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0A );    // Work - Single channel digital potentiometer
auto ds2890B  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0B );
auto ds2890C  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0C );
auto bae910   = BAE910(BAE910::family_code, 0x00, 0x00, 0x10, 0xE9, 0xBA, 0x00);

int main()
{
    cout << "Hello, World!" << endl;
    
    // Setup OneWire
    hubA.attach(ds1822);
    hubA.attach(ds18B20);
    hubA.attach(ds18S20);
    hubA.attach(ds2401a);
    hubA.attach(ds2401b);
    hubA.attach(ds2405);
    hubA.attach(ds2408);
    hubA.attach(ds2413);

    hubB.attach(ds2423);
    hubB.attach(ds2431);
    hubB.attach(ds2433);
    hubB.attach(ds2438);
    hubB.attach(ds2450);
    hubB.attach(ds2502);
    hubB.attach(ds2501a);
    hubB.attach(ds2501b);

    hubC.attach(ds2503);
    hubC.attach(ds2505);
    hubC.attach(ds2506);
    hubC.attach(ds2890A);
    hubC.attach(ds2890B);
    hubC.attach(ds2890C);
    hubC.attach(bae910);


    ds1822.setTemperature(static_cast<int8_t>(21));
    ds18S20.setTemperature(static_cast<int8_t>(10));

    hubA.poll();
    hubB.poll();
    hubC.poll();

    if (hubA.hasError()) hubA.printError();
    if (hubB.hasError()) hubB.printError();
    if (hubC.hasError()) hubC.printError();

    return 0;
};