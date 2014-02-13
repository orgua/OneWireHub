#include "OneWireHub.h"
//#include "DS2401.h"  // Serial Number
//#include "DS18B20.h" // Digital Thermometer
//#include "DS2405.h"  // Single adress switch
//#include "DS2408.h"  // 8-Channel Addressable Switch
//#include "DS2413.h"  // Dual channel addressable switch
//#include "DS2423.h"  // 4kb 1-Wire RAM with Counter
//#include "DS2433.h"  // 4Kb 1-Wire EEPROM
#include "DS2438.h"  // Smart Battery Monitor
//#include "DS2450.h"  // 4 channel A/D
//#include "DS2890.h"  // Single channel digital panemtiometer

const int ledPin =  13;         // the number of the LED pin
// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated
long interval = 250;           // interval at which to blink (milliseconds) 

OneWireHub * hub = 0;

void blinking() {
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) {
    // save the last time you blinked the LED 
    previousMillis = currentMillis; 
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    digitalWrite(ledPin, ledState);
  }
}

void setup() {
  // Debug
  Serial.begin(9600);
  
  hub = new OneWireHub(8);

  // put your setup code here, to run once:
//  hub->elms[0] = new DS18B20( 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Digital Thermometer  
//  hub->elms[1] = new DS2401 ( 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Serial Number
//  hub->elms[2] = new DS2405(  0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Single adress switch
//  hub->elms[3] = new DS2408(  0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 8-Channel Addressable Switch
//  hub->elms[4] = new DS2413(  0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Dual channel addressable switch
//  hub->elms[5] = new DS2423(  0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
//  hub->elms[6] = new DS2433(  0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4kb 1-Wire RAM with Counter
  hub->elms[7] = new DS2438(  0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - Smart Battery Monitor
//  hub->elms[0] = new DS2450(  0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    //      - 4 channel A/D
//  hub->elms[1] = new DS2890(  0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 );    // Work - Single channel digital panemtiometer

  Serial.println( hub->calck_mask() );
}

void loop() {
  // put your main code here, to run repeatedly: 
  hub->waitForRequest(false);
  
  // Blink
  blinking();
}
