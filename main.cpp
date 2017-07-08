//
//
//
#include <iostream>
#include <vector>

using namespace std;

#include "src/platform.h"
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

constexpr uint8_t operator "" _u8(const unsigned long long int value)
{
    return static_cast<uint8_t>(value);
}

constexpr int8_t operator "" _i8(const unsigned long long int value)
{
    return static_cast<int8_t>(value);
}

size_t tests_absolved = 0;

template<typename T1, typename T2>
void test_eq(const T1 value_A, const T2 value_B, const string message)
{
    if (value_A != value_B) cout << "- FAIL (" << to_string(value_A) << " != " << to_string(value_B) << ") " << message << endl;
    tests_absolved++;
}

template<typename T1>
const vector<T1> initializeLinear(const T1 value_start, const T1 value_increment, const size_t size)
{
    vector<T1> data(size,value_start);
    T1 value = value_start;
    for (size_t index = 0; index < size; ++index)
    {
        data[index] = value;
        value += value_increment;
    }
    return data;
}


int main()
{
    cout << "- initialize all devices" << endl;

    constexpr uint8_t OneWire_PIN   = 8;

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

    auto ds2401c  = DS2401( 0x01, 0x00, 0x0D, 0x24, 0x01, 0x00, 0x0C );    // additional device for testing

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

    cout << "- attach devices to hubs" << endl;
    
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

    cout << "- use every device-function at least once and do some unit-testing" << endl;

    // TODO: maybe put the code in src_files to the depending device_unittest.h

    {
        /// DS18B20
        const auto temp_A = initializeLinear( -55.0f, 1.0f, 181);
        const auto temp_B = initializeLinear( int8_t(-55), 1_i8, 181);

        for (const auto temp : temp_A)
        {
            ds1822.setTemperature(temp);
            test_eq(ds1822.getTemperature(), temp, "DS1822 float temp =" + to_string(temp));
        }

        ds1822.setTemperature(-56.0f);
        test_eq(ds1822.getTemperature(), -55.0f, "DS1822 float out of bounds NEG");

        ds1822.setTemperature(126.0f);
        test_eq(ds1822.getTemperature(), 125.0f, "DS1822 float out of bounds POS");

        for (const auto temp : temp_B)
        {
            ds18B20.setTemperature(temp);
            test_eq(ds18B20.getTemperature(), temp, "DS18B22 int8 temp =" + to_string(temp));
        }

        for (const auto temp : temp_B)
        {
            ds18S20.setTemperature(temp);
            test_eq(ds18S20.getTemperature(), temp, "DS18S22 int8 temp =" + to_string(temp));
        }
    }

    {
        /// DS2401 and general hub test
        const auto position_A = hubA.attach(ds2401c);
        test_eq(position_A, 255_u8, "DS2401 attach to full hub");

        const auto position_B = hubA.detach(ds2401c);
        test_eq(position_B, false, "DS2401 detach not attached device to full hub");

        const auto position_C = hubA.detach(ds2401a);
        test_eq(position_C, true, "DS2401 detach an attached device");

        const auto position_D = hubA.attach(ds2401b);
        test_eq(position_D, 4_u8, "DS2401 attach an already attached device");

        const auto position_E = hubA.attach(ds2401c);
        test_eq(position_E, 3_u8, "DS2401 attach an unattached devices");
    }

    // TODO: look through src code, for now just converted ino-examples

    {
        /// DS2405
        ds2405.setPinState(true);
        test_eq(ds2405.getPinState(), true, "DS2405 true");
        ds2405.setPinState(false);
        test_eq(ds2405.getPinState(), false, "DS2405 false");
    }

    {
        /// DS2408 TODO: vector with tests, raise activity
        ds2408.clearMemory();
        ds2408.setPinState(0, true);
        ds2408.setPinState(1, false);
        ds2408.setPinState(2, false);
        ds2408.setPinState(3, false);
        test_eq(ds2408.getPinState(0), true, "DS2408 state true");
        test_eq(ds2408.getPinState(1), false, "DS2408 state false");
        test_eq(ds2408.getPinState() & 0x0F, 0x01, "DS2408 state cplx");

        ds2408.setPinActivity(0, true);
        ds2408.setPinActivity(1, false);
        ds2408.setPinActivity(2, false);
        ds2408.setPinActivity(3, false);
        test_eq(ds2408.getPinActivity(0), true, "DS2408 activity true");
        test_eq(ds2408.getPinActivity(1), false, "DS2408 activity false");
        test_eq(ds2408.getPinActivity() & 0x0F, 0x01, "DS2408 activity cplx");
    }

    {
        /// DS2413
        const auto values_pin = initializeLinear(0_u8, 1_u8, 2);

        for (const auto value : values_pin)
        {
            ds2413.setPinState(value,false);
            test_eq(ds2413.getPinState(value), false, "DS2413 state of pin " + to_string(value));

            ds2413.setPinState(value,true);
            test_eq(ds2413.getPinState(value), true, "DS2413 state of pin " + to_string(value));

            ds2413.setPinLatch(value,false);
            test_eq(ds2413.getPinLatch(value), false, "DS2413 latch of pin " + to_string(value));

            ds2413.setPinLatch(value,true);
            test_eq(ds2413.getPinLatch(value), true, "DS2413 latch of pin " + to_string(value));

            test_eq(ds2413.getPinState(value), false, "DS2413 change in state of pin " + to_string(value) + " because of latch");
            ds2413.setPinState(value,true);
            test_eq(ds2413.getPinState(value), false, "DS2413 re-enable state of pin " + to_string(value) + " fails because pin still latched");

            ds2413.setPinLatch(value,false);
            test_eq(ds2413.getPinState(value), false, "DS2413 state of pin " + to_string(value) + " after disabling latch");

            ds2413.setPinState(value,true);
            test_eq(ds2413.getPinState(value), true, "DS2413 re-enable state of pin " + to_string(value) + " after disabling latch works");
        }
    }

    {
        // DS2423
        constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
        constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

        ds2423.clearMemory(); // begin fresh after doing some work

        test_eq(ds2423.getCounter(0), 0, "DS2423 write counter 0 - fresh state");
        ds2423.writeMemory(mem_dummy, sizeof(mem_dummy), 12*32+16); // second half of page 12
        test_eq(ds2423.getCounter(0), 1, "DS2423 write counter 0 - increment by writing register");

        test_eq(ds2423.getCounter(1), 0, "DS2423 write counter 1 - fresh state");
        ds2423.setCounter(1,2000);
        test_eq(ds2423.getCounter(1), 2000, "DS2423 write counter 1 - set counter");

        ds2423.writeMemory(mem_dummy, sizeof(mem_dummy), 12*32+17); // second half of page 12 and 1 byte of page 13
        test_eq(ds2423.getCounter(0), 2, "DS2423 write counter 0 - increment by writing register");
        test_eq(ds2423.getCounter(1), 2001, "DS2423 write counter 1 - increment by writing register");

        test_eq(ds2423.getCounter(2), 0, "DS2423 write counter 2 - fresh state");
        ds2423.incrementCounter(2);
        test_eq(ds2423.getCounter(2), 1, "DS2423 write counter 2 - incrementing");
        ds2423.decrementCounter(2);
        test_eq(ds2423.getCounter(2), 0, "DS2423 write counter 2 - decrementing");

        test_eq(ds2423.getCounter(3), 0, "DS2423 write counter 3 - fresh state");

        // Test-Cases: the following code is just to show basic functions, can be removed any time
        ds2423.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),0x00);

        ds2423.writeMemory(mem_dummy, sizeof(mem_dummy), 1*32);

        uint8_t mem_read[16];
        ds2423.readMemory(mem_read, 16, 31); // begin one byte earlier than page 1

        for (size_t index = 1; index < sizeof(mem_dummy); ++index)
        {
            test_eq(mem_read[index], mem_dummy[index-1], "DS2423 mem re-read at position " + to_string(index));
        }
    }

    {
        // DS2431
        constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
        constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

        ds2431.clearMemory(); // begin fresh after doing some work

        ds2431.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),0x00);
        ds2431.writeMemory(mem_dummy, sizeof(mem_dummy), 1*32);

        uint8_t mem_read[16];
        ds2431.readMemory(mem_read, 16, 31); // begin one byte earlier than page 1
        for (size_t index = 1; index < sizeof(mem_dummy); ++index)
        {
            test_eq(mem_read[index], mem_dummy[index-1], "DS2431 mem re-read at position " + to_string(index));
        }

        test_eq(ds2431.getPageProtection(1*32 -  1), false, "DS2431 test page protection before");
        test_eq(ds2431.getPageProtection(1*32 +  0), false, "DS2431 test page protection before");
        test_eq(ds2431.getPageProtection(2*32 +  0), false, "DS2431 test page protection before");
        ds2431.setPageProtection(1*32);
        test_eq(ds2431.getPageProtection(1*32 -  1), false, "DS2431 test page protection after");
        test_eq(ds2431.getPageProtection(1*32 +  0),  true, "DS2431 test page protection after");
        test_eq(ds2431.getPageProtection(1*32 +  8),  true, "DS2431 test page protection after");
        test_eq(ds2431.getPageProtection(1*32 + 18),  true, "DS2431 test page protection after");
        test_eq(ds2431.getPageProtection(2*32 -  1),  true, "DS2431 test page protection after");
        test_eq(ds2431.getPageProtection(2*32 -  0), false, "DS2431 test page protection after");
        ds2431.setPageProtection(2*32);
        test_eq(ds2431.getPageProtection(2*32 -  0),  true, "DS2431 test page protection after");


        constexpr uint8_t mem_FF[] = { 0xFF, 0xFF };
        ds2431.writeMemory(reinterpret_cast<const uint8_t *>(mem_FF),sizeof(mem_FF),2*32);
        test_eq(ds2431.getPageEpromMode(2*32 - 1), false, "DS2431 test page eprom mode before");
        test_eq(ds2431.getPageEpromMode(2*32 - 0), false, "DS2431 test page eprom mode before");
        test_eq(ds2431.getPageEpromMode(2*32 + 1), false, "DS2431 test page eprom mode before");
        ds2431.setPageEpromMode(2*32);
        test_eq(ds2431.getPageEpromMode(2*32 - 1), false, "DS2431 test page eprom mode after");
        test_eq(ds2431.getPageEpromMode(2*32 - 0),  true, "DS2431 test page eprom mode after");
        test_eq(ds2431.getPageEpromMode(2*32 + 1),  true, "DS2431 test page eprom mode after");
        test_eq(ds2431.getPageEpromMode(3*32 - 1),  true, "DS2431 test page eprom mode after");
        test_eq(ds2431.getPageEpromMode(3*32 - 0), false, "DS2431 test page eprom mode after");

        // TODO: test real eprom
    }



    bae910.memory.field.rtc = 1000;

    cout << "- poll the hubs" << endl;

    hubA.poll();
    hubB.poll();
    hubC.poll();

    if (hubA.hasError()) hubA.printError();
    if (hubB.hasError()) hubB.printError();
    if (hubC.hasError()) hubC.printError();

    cout << "Program did run " << tests_absolved << " tests." << endl;

    return 0;
};