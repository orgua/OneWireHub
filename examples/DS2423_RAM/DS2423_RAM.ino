/*
*    Example-Code that emulates a DS2423 4096 bits RAM with Counter
*
*   THIS DEVICE IS ONLY A MOCKUP FOR NOW - NO REAL FUNCTION
*
*   Tested with
*   - DS9490R-Master, atmega328@16MHz and teensy3.2@96MHz as Slave
*/

#include "OneWireHub.h"
#include "DS2423.h"

constexpr uint8_t pin_onewire   { 8 };

auto hub = OneWireHub(pin_onewire);
auto ds2423 = DS2423(DS2423::family_code, 0x00, 0x00, 0x23, 0x24, 0xDA, 0x00);

void setup()
{
    Serial.begin(115200);
    Serial.println("OneWire-Hub DS2423");

    // Setup OneWire
    hub.attach(ds2423);

    // Test-Cases: the following code is just to show basic functions, can be removed any time
    Serial.println("Test Write Text Data to page 0");
    constexpr char memory[] = "abcdefg-test-data full ASCII:-?+";
    ds2423.writeMemory(reinterpret_cast<const uint8_t *>(memory),sizeof(memory),0x00);

    Serial.println("Test Write binary Data to page 1");
    constexpr uint8_t mem_dummy[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    ds2423.writeMemory(mem_dummy, sizeof(mem_dummy), 1*32);

    Serial.print("Test Read binary Data to page 1: 0x");
    uint8_t mem_read[16];
    ds2423.readMemory(mem_read, 16, 31); // begin one byte earlier than page 1
    Serial.println(mem_read[2],HEX); // should read 0x11

    Serial.println("Test Write binary Data to page 12 and influence counter0");
    Serial.println(ds2423.getCounter(0));
    ds2423.writeMemory(mem_dummy, sizeof(mem_dummy), 12*32+16); // second half of page 12
    Serial.println(ds2423.getCounter(0));

    Serial.println("Test Read and write Counter 1: ");
    Serial.println(ds2423.getCounter(1));
    ds2423.setCounter(1,2000);
    Serial.println(ds2423.getCounter(1));

    Serial.println("Test Inc and Decrement Counter 2: ");
    Serial.println(ds2423.getCounter(2));
    ds2423.incrementCounter(2);
    Serial.println(ds2423.getCounter(2));
    ds2423.decrementCounter(2);
    Serial.println(ds2423.getCounter(2));

    Serial.println("Test Read all Counters");
    Serial.println(ds2423.getCounter(0));
    Serial.println(ds2423.getCounter(1));
    Serial.println(ds2423.getCounter(2));
    Serial.println(ds2423.getCounter(3));

    // ds2423.clearMemory(); // begin fresh after doing some work

    Serial.println("config done");
}

void loop()
{
    // following function must be called periodically
    hub.poll();
} 