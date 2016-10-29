#ifndef ONEWIREHUB_CONFIG_H_H
#define ONEWIREHUB_CONFIG_H_H

/////////////////////////////////////////////////////
// CONFIG ///////////////////////////////////////////
/////////////////////////////////////////////////////

// INFO: had to go with a define because some compilers use constexpr as simple const --> massive problems
#define HUB_SLAVE_LIMIT     8 // set the limit of the hub HERE, max is 32 devices
#define OVERDRIVE_ENABLE    0 // support overdrive for the slaves

constexpr bool     USE_SERIAL_DEBUG { 0 }; // give debug messages when printError() is called (be aware! it may produce heisenbugs, timing is critical)
constexpr bool     USE_GPIO_DEBUG   { 0 }; // is a better alternative to serial debug (see readme.md for info)
constexpr uint8_t  GPIO_DEBUG_PIN   { 7 }; // digital pin
constexpr uint32_t REPETITIONS      { 5000 }; // for measuring the loop-delay --> 10000L takes ~110ms on atmega328p@16Mhz

/// the following TIME-values are in microseconds and are taken from the ds2408 datasheet
// should be --> datasheet
// was       --> shagrat-legacy
constexpr uint16_t ONEWIRE_TIME_RESET_TIMEOUT        = 5000; // for not hanging indef in reset-detection, lower value is better for more responsive applications, but can miss resets
constexpr uint16_t ONEWIRE_TIME_RESET_MIN            =  430; // should be 480, and was 470
constexpr uint16_t ONEWIRE_TIME_RESET_MAX            =  960; // from ds2413

constexpr uint16_t ONEWIRE_TIME_PRESENCE_TIMEOUT     =   20; // probe measures 40us
constexpr uint16_t ONEWIRE_TIME_PRESENCE_MIN         =  160; // was 125
constexpr uint16_t ONEWIRE_TIME_PRESENCE_MAX         =  480; // should be 280, was 480 !!!! why

constexpr uint16_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     =15000; // there can be inactive / high timeperiods after reset / presence, this value defines the timeout for these
constexpr uint16_t ONEWIRE_TIME_SLOT_MAX             =  135; // should be 120, was ~1050

// read and write from the viewpoint of the slave!!!!
constexpr uint16_t ONEWIRE_TIME_READ_MIN             =   20; // was 30, should be 15
constexpr uint16_t ONEWIRE_TIME_READ_MAX             =   60; //
constexpr uint16_t ONEWIRE_TIME_WRITE_ZERO           =   30; //

// OVERDRIVE
constexpr uint16_t OVERDRIVE_TIME_RESET_MIN          =   48; //
constexpr uint16_t OVERDRIVE_TIME_RESET_MAX          =   80; //
//

constexpr uint16_t OVERDRIVE_TIME_PRESENCE_TIMEOUT   =   20;
constexpr uint16_t OVERDRIVE_TIME_PRESENCE_MIN       =    8;
constexpr uint16_t OVERDRIVE_TIME_PRESENCE_MAX       =   32;
//

constexpr uint16_t OVERDRIVE_TIME_SLOT_MAX           =   30; //

constexpr uint16_t OVERDRIVE_TIME_READ_MIN           =    4; //
constexpr uint16_t OVERDRIVE_TIME_READ_MAX           =   10; //
constexpr uint16_t OVERDRIVE_TIME_WRITE_ZERO         =    8; //

// VALUES FOR STATIC ASSERTS
constexpr uint16_t ONEWIRE_TIME_VALUE_MAX            = ONEWIRE_TIME_MSG_HIGH_TIMEOUT;
#if OVERDRIVE_ENABLE
constexpr uint16_t ONEWIRE_TIME_VALUE_MIN            = OVERDRIVE_TIME_READ_MIN;
#else
constexpr uint16_t ONEWIRE_TIME_VALUE_MIN            = ONEWIRE_TIME_PRESENCE_TIMEOUT;
#endif

/////////////////////////////////////////////////////
// END OF CONFIG ////////////////////////////////////
/////////////////////////////////////////////////////

#endif //ONEWIREHUB_CONFIG_H_H
