//
//
//
#include <iostream>

using namespace std;

#include "OneWireHub.h"

// include all libs to find errors
#include "DS2401.h"  // Serial Number
#include "DS18B20.h" // Digital Thermometer
#include "DS2405.h"  // Single adress switch
#include "DS2408.h"  // 8-Channel Addressable Switch
#include "DS2413.h"  // Dual channel addressable switch
#include "DS2423.h"  // 4kb 1-Wire RAM with Counter
#include "DS2431.h"  // 1kb 1-Wire EEPROM
#include "DS2433.h"  // 4Kb 1-Wire EEPROM
#include "DS2438.h"  // Smart Battery Monitor
#include "DS2450.h"  // 4 channel A/D
#include "DS2502.h"  // 1kb EEPROM
#include "DS2890.h"  // Single channel digital potentiometer



// taken from OneWireHubTest.ino

const uint8_t OneWire_PIN   = 8;

auto hub      = OneWireHub(OneWire_PIN);
auto ds1822   = DS18B20(0x22, 0x0D, 0x01, 0x08, 0x02, 0x00, 0x00);
auto ds18B20  = DS18B20(0x28, 0x0D, 0x01, 0x08, 0x0B, 0x02, 0x00);      // Work - Digital Thermometer
auto ds18S20  = DS18B20(0x10, 0x0D, 0x01, 0x08, 0x0F, 0x02, 0x00);
auto ds2401a  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0A );    // Work - Serial Number
auto ds2401b  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0B );    // Work - Serial Number
// auto ds2405   = DS2405( 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Single address switch
// auto ds2408   = DS2408( 0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 8-Channel Addressable Switch
auto ds2413   = DS2413( 0x3A, 0x0D, 0x02, 0x04, 0x01, 0x03, 0x00 );    // Work - Dual channel addressable switch
// auto ds2423   = DS2423( 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
auto ds2431   = DS2431( 0x2D, 0xE8, 0x9F, 0x90, 0x0E, 0x00, 0x00 );    // Work - 1kb 1-Wire EEPROM
// auto ds2433   = DS2433( 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4Kb 1-Wire EEPROM
auto ds2438   = DS2438( 0x26, 0x0D, 0x02, 0x04, 0x03, 0x08, 0x00 );    //      - Smart Battery Monitor
auto ds2450   = DS2450( 0x20, 0x0D, 0x0A, 0x02, 0x04, 0x05, 0x00 );    //      - 4 channel A/D
auto ds2890A  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0A );    // Work - Single channel digital potentiometer
auto ds2890B  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0B );
auto ds2890C  = DS2890( 0x2C, 0x0D, 0x02, 0x08, 0x09, 0x00, 0x0C );

int main()
{
    cout << "Hello, World!" << endl;
    
    // Setup OneWire
    ds1822.setTemp(static_cast<int16_t>(21));
    ds18S20.setTemp(static_cast<int16_t>(10));
    hub.attach(ds1822);
    hub.attach(ds18B20);
    hub.attach(ds18S20);
    hub.attach(ds2401a);
    hub.attach(ds2401b);
    hub.attach(ds2413);
    hub.attach(ds2431);
    hub.attach(ds2438);
    //hub.attach(ds2450);
    hub.attach(ds2890A);
    hub.attach(ds2890B);
    hub.attach(ds2890C);

    hub.poll();

    if (hub.getError()) hub.printError();

    return 0;
};