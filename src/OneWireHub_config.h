#ifndef ONEWIREHUB_CONFIG_H_H
#define ONEWIREHUB_CONFIG_H_H

#include "platform.h"
/////////////////////////////////////////////////////
// CONFIG ///////////////////////////////////////////
/////////////////////////////////////////////////////

// INFO: had to go with a define because some compilers use constexpr as simple const --> massive problems
#define HUB_SLAVE_LIMIT     8 // set the limit of the hub HERE, max is 32 devices
#define OVERDRIVE_ENABLE    0 // support overdrive for the slaves

constexpr bool     USE_SERIAL_DEBUG { false }; // give debug messages when printError() is called (be aware! it may produce heisenbugs, timing is critical) SHOULD NOT be enabled with < 20 MHz uC
constexpr bool     USE_GPIO_DEBUG   { false }; // is a better alternative to serial debug (see readme.md for info) SHOULD NOT be enabled with < 20 MHz uC and Overdrive enabled
constexpr uint8_t  GPIO_DEBUG_PIN   { 7 }; // digital pin
constexpr uint32_t REPETITIONS      { 5000 }; // for measuring the loop-delay --> 10000L takes ~110ms on atmega328p@16Mhz

static_assert(!(USE_SERIAL_DEBUG && (microsecondsToClockCycles(1) < 20)), "Serial debug is enabled in OW-Config. SHOULD NOT be enabled with < 20 MHz uC");
static_assert(!(USE_GPIO_DEBUG && (microsecondsToClockCycles(1) < 20) && (OVERDRIVE_ENABLE != 0)), "Gpio debug is enabled in OW-Config. SHOULD NOT be enabled with < 20 MHz uC and Overdrive enabled");

/// the following TIME-values are in microseconds and are taken mostly from the ds2408 datasheet
//  arrays contain the normal timing value and the overdrive-value, the literal "_us" converts the value right away to a usable unit
//  should be --> datasheet
//  was       --> shagrat-legacy

// Reset: every low-state of the master between MIN & MAX microseconds will be recognized as a Reset
constexpr timeOW_t ONEWIRE_TIME_RESET_TIMEOUT        = {  5000_us };        // for not hanging to long in reset-detection, lower value is better for more responsive applications, but can miss resets
constexpr timeOW_t ONEWIRE_TIME_RESET_MIN[2]         = {   430_us, 48_us }; // should be 480
constexpr timeOW_t ONEWIRE_TIME_RESET_MAX[2]         = {   960_us, 80_us }; // from ds2413

// Presence: slave waits TIMEOUT and emits a low state after the reset with ~MIN length, if the bus stays low after that and exceeds MAX the hub will issue an error
constexpr timeOW_t ONEWIRE_TIME_PRESENCE_TIMEOUT     = {    20_us };        // probe measures 25us, duration of high state between reset and presence
constexpr timeOW_t ONEWIRE_TIME_PRESENCE_MIN[2]      = {   160_us,  8_us }; // was 125
constexpr timeOW_t ONEWIRE_TIME_PRESENCE_MAX[2]      = {   480_us, 32_us }; // should be 280, was 480


constexpr timeOW_t ONEWIRE_TIME_MSG_HIGH_TIMEOUT     = { 15000_us };        // there can be these inactive / high timeperiods after reset / presence, this value defines the timeout for these
constexpr timeOW_t ONEWIRE_TIME_SLOT_MAX[2]          = {   135_us, 30_us }; // should be 120, measured from falling edge to next falling edge

// read and write from the viewpoint of the slave!!!!
constexpr timeOW_t ONEWIRE_TIME_READ_MIN[2]          = {    20_us,  4_us }; // should be 15, was 30, says when it is safe to read a valid bit
constexpr timeOW_t ONEWIRE_TIME_READ_MAX[2]          = {    60_us, 10_us }; // low states (zeros) of a master should not exceed this time in a slot
constexpr timeOW_t ONEWIRE_TIME_WRITE_ZERO[2]        = {    30_us,  8_us }; // the hub holds a zero for this long

// VALUES FOR STATIC ASSERTS
constexpr timeOW_t ONEWIRE_TIME_VALUE_MAX            = { ONEWIRE_TIME_MSG_HIGH_TIMEOUT };
constexpr timeOW_t ONEWIRE_TIME_VALUE_MIN            = { ONEWIRE_TIME_READ_MIN[OVERDRIVE_ENABLE] };

// TODO: several compilers have problems with constexpress-FN in unified initializers of constexpr, will be removed for now -> test with arduino due, esp32, ...

/////////////////////////////////////////////////////
// END OF CONFIG ////////////////////////////////////
/////////////////////////////////////////////////////

#endif //ONEWIREHUB_CONFIG_H_H
