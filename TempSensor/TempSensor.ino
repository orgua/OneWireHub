#include "OneWireHub.h"
#include "DS18B20.h"  // Dual channel addressable switch

#define OneWire_PIN 8

OneWireHub * hub = 0;

DS18B20 * fMS; 

void setup() {
  // Debug
  Serial.begin(9600);
    
  // Work - Digital Thermometer
  fMS = new DS18B20(  0x28, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 );    

  // Setup OneWire
  hub = new OneWireHub( OneWire_PIN ); 
  hub->elms[0] = fMS;
  hub->calck_mask();
}

void loop() {
  // Set temp
  fMS->settemp(20.1);
  
  // put your main code here, to run repeatedly:
  hub->waitForRequest(false); 
}