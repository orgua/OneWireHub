/*
 *    Test-Code for a suitable timing function
 *
 *      --> try to determine bus-timing by observing the master
 *
 */

#include "OneWireHub.h"

constexpr uint8_t pin_led       { 13 }; // TODO: take this code to other examples
constexpr uint8_t pin_onewire   { 8 };

constexpr uint32_t repetitions  { 5000 }; // how many low_states will measured before assuming that there was a reset in it
constexpr uint32_t value1k      { 1000 };
constexpr uint32_t wait_retries { 1000000 * microsecondsToClockCycles(1) }; // loops before cancelling a pin-change-wait

auto hub    = OneWireHub(pin_onewire);


/////////////////////////////////////////////////////////////////////////
/////// Main Code              //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

timeOW_t retries_500ms = 0;

void setup()
{
    pinMode(pin_led, OUTPUT);
    Serial.begin(115200);

    uint32_t time_max = 0;
    uint32_t  measure = 10;

    // measure the longest low-states on the bus with millis(), assume it is a reset
    while(measure--)
    {
        uint32_t time_needed = 0;

        // try to catch a reset
        while (time_needed < ONEWIRE_TIME_RESET_MIN)
        {
            hub.delayLoopsWhilePinIs(wait_retries, true);
            const uint32_t time_start = micros();
            hub.waitWhilePinIs(false);
            const uint32_t time_stop = micros();
            time_needed = time_stop - time_start;
        };

        if (time_needed>time_max) time_max = time_needed;
    };
    Serial.print(time_max);
    Serial.println("\t us per reset max");
    Serial.flush();


    timeOW_t retries_max = 0;
    measure = 0;

    while(measure++ < repetitions)
    {
        hub.delayLoopsWhilePinIs(wait_retries, true);
        const timeOW_t retries_needed = hub.measureLoopsWhilePinIs(false);
        if (retries_needed>retries_max) retries_max = retries_needed;
    };
    Serial.print(retries_max);
    Serial.println("\t loops per reset max");

    const timeOW_t factor_ipl = time_max * microsecondsToClockCycles(1) / retries_max;

    Serial.print(factor_ipl);
    Serial.println("\t instructions per loop");

    const timeOW_t factor_nspl = time_max * value1k / retries_max;

    retries_500ms = 500 * value1k * value1k / factor_nspl;
};

void loop()
{
    digitalWrite(pin_led, true);
    hub.delayLoopsWhilePinIs(retries_500ms, true);


}
