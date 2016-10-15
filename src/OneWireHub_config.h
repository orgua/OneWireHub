#ifndef ONEWIREHUB_ONEWIREHUB_CONFIG_H_H
#define ONEWIREHUB_ONEWIREHUB_CONFIG_H_H

/////////////////////////////////////////////////////
// CONFIG ///////////////////////////////////////////
/////////////////////////////////////////////////////

// INFO: had to go with a define because some compilers use constexpr as simple const --> massive problems
#define HUB_SLAVE_LIMIT     8 // set the limit of the hub HERE
#define OVERDRIVE_ENABLE    0 // support overdrive for the slaves
#define CALIBRATION_ENABLE  0 // support active calibration by observing the onewire-master for some time during startup

#define USE_SERIAL_DEBUG    0 // give debug messages when printError() is called
#define USE_GPIO_DEBUG      1

constexpr uint8_t  GPIO_DEBUG_PIN   { 7 }; // digital pin
constexpr uint32_t REPETITIONS      { 10000 }; // for measuring the loop-delay --> 10000L take ~110ms on atmega328p@16Mhz

/// the following TIME-values are in microseconds and are taken from the ds2408 datasheet
// should be --> datasheet
// was       --> shagrat-legacy
constexpr uint16_t ONEWIRE_TIME_BUS_CHANGE_MAX       =    5; //

constexpr uint16_t ONEWIRE_TIME_RESET_MIN            =  430; // should be 480, and was 470
constexpr uint16_t ONEWIRE_TIME_RESET_MAX            =  960; // from ds2413
constexpr uint16_t ONEWIRE_TIME_RESET_TIMEOUT        =10000; // for not hanging indef in reset-detection,  // TODO: should optimize, lower value is better

constexpr uint16_t ONEWIRE_TIME_PRESENCE_SAMPLE_MIN  =   20; // probe measures 40us
constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_STD     =  160; // was 125
constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MAX     =  480; // should be 280, was 480 !!!! why
constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MAX    =20000; // TODO: length of high-side not really relevant, so we should switch to a fn that detects the length of the most recent low-phase

constexpr uint16_t ONEWIRE_TIME_SLOT_MAX             =  135; // should be 120, was ~1050

// read and write from the viewpoint of the slave!!!!
constexpr uint16_t ONEWIRE_TIME_READ_ONE_LOW_MAX     =   60; //
constexpr uint16_t ONEWIRE_TIME_READ_STD             =   20; // was 30
constexpr uint16_t ONEWIRE_TIME_WRITE_ZERO_LOW_STD   =   35; //

/////////////////////////////////////////////////////
// END OF CONFIG ////////////////////////////////////
/////////////////////////////////////////////////////

#endif //ONEWIREHUB_ONEWIREHUB_CONFIG_H_H
