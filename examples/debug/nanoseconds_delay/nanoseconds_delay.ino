/*
 *    Test-Code for a suitable timing function
 *
 *    - Var1: use micros()
 *    - Var2: count ticks
 *
 *    Results (Atmega328@16MHz)
 *    - 16bit-ticks-loop takes 11 ticks to repeat, 32bit takes 14 ticks
 *    - results on pin are the same, early leaving the loop takes 690 ns
 *    -
 *
 */

#include "OneWireHub.h"

class WaitWhilePinIs
{
private:
    // setup direct pin-access
    uint8_t pin_bitMask;
    volatile uint8_t *pin_baseReg;
    //volatile uint8_t *reg asm("r30") = baseReg;

    uint32_t factor_nslp;

public:
    WaitWhilePinIs(uint8_t digital_pin) // initialize and calibrate
    {
        // initialize
        pin_bitMask  = digitalPinToBitMask(digital_pin);
        pin_baseReg = portInputRegister(digitalPinToPort(digital_pin));

        // prepare measurement
        DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);
        DIRECT_WRITE_HIGH(pin_baseReg, pin_bitMask);
        uint8_t pin_value = 1;
        static_assert(microsecondsToClockCycles(1) < 40000L, "CPU is too fast"); // protect from overrun with static_assert, maybe convert to dynanic type
        const uint32_t retries = 100000L * microsecondsToClockCycles(1); // 100ms (if loop takes 1 cylce) for every frequency, uint32-overrun at 40 GHz
        // 100000L takes 1100ms on atmega328p@16Mhz

        // measure
        const uint32_t time_start = micros();
        loops(retries,pin_value);
        const uint32_t time_stop = micros();

        // analyze
        const uint32_t time_ns = (time_stop - time_start) * 1000;
        factor_nslp = time_ns / retries; // nanoseconds per loop
        DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask); // disable internal pullup
        DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
    };

    void loops(volatile uint32_t retries, const bool pin_value = false)
    {
        while (DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value) if (--retries == 0) break; // standard loop for measuring, 13 cycles per loop32 for an atmega328p
    };

    uint32_t calculateRetries(const uint32_t time_ns)
    {
        return (time_ns / factor_nslp); // precalc waitvalues, the OP can take up da 550 cylces
    };

    void nanoseconds(const uint32_t time_ns, const bool pin_value = false)
    {
        if (time_ns < factor_nslp) return;
        uint32_t retries = calculateRetries(time_ns); // not cheap .... precalc if possible
        loops(retries, pin_value);
    };
};

class Delay
{
private:
    uint32_t factor_nslp;

public:
    Delay(void) // initialize and calibrate
    {
        // prepare measurement
        static_assert(microsecondsToClockCycles(1) < 40000L, "CPU is too fast"); // protect from overrun with static_assert, maybe convert to dynanic type
        const uint32_t retries = 100000L * microsecondsToClockCycles(1); // 100ms (if loop takes 1 cylce) for every frequency, uint32-overrun at 40 GHz
        // 100000L takes 600ms on atmega328p@16Mhz

        // measure
        const uint32_t time_start = micros();
        loops(retries);
        const uint32_t time_stop = micros();

        // analyze
        const uint32_t time_ns = (time_stop - time_start) * 1000;
        factor_nslp = time_ns / retries; // nanoseconds per loop
    };

    void loops(volatile uint32_t retries)
    {
        while (--retries); // standard loop for measuring
    };

    uint32_t calculateRetries(const uint32_t time_ns)
    {
        return (time_ns / factor_nslp); // precalc waitvalues, the OP can take up da 550 cylces
    };

    void nanoseconds(const uint32_t time_ns)
    {
        if (time_ns < factor_nslp) return;
        const uint32_t retries = calculateRetries(time_ns); // not cheap .... precalc if possible
        loops(retries);
    };
};



void setup()
{
    const uint8_t pin_debug = 2;
    const uint8_t pin_delay = 8;
    const bool    pin_value = false;

    const uint32_t wait_ns[] = { 1000, 10000, 100000, 1000000}; // 1us, 10us, 100us, 1ms
    const uint8_t  sizeof_wait = sizeof(wait_ns) >> 2;

    pinMode(pin_debug,OUTPUT);
    // TODO: measure with logic analyzer and test if values are ok, pin_delay has to be false, otherwise dalay will exit early

    { // just delay
        digitalWrite(pin_debug,HIGH);
        auto delay32  = Delay();
        digitalWrite(pin_debug,LOW);
        delay(5);

        for (uint8_t i = 0; i < sizeof_wait; ++i)
        {
            const uint32_t retries = delay32.calculateRetries(wait_ns[i]);

            digitalWrite(pin_debug, HIGH);
            delay32.loops(retries);
            digitalWrite(pin_debug, LOW);

            delay(5);
        };
    }; // got: <5us, 13.4us, 103.6us, 1005us

    delay(10);

    { // do a pincheck
        digitalWrite(pin_debug,HIGH);
        auto delay32  = WaitWhilePinIs(pin_delay);
        digitalWrite(pin_debug,LOW);
        delay(5);

        for (uint8_t i = 0; i < sizeof_wait; ++i)
        {
            const uint32_t retries = delay32.calculateRetries(wait_ns[i]);

            digitalWrite(pin_debug, HIGH);
            delay32.loops(retries, pin_value);
            digitalWrite(pin_debug, LOW);

            delay(5);
        };
    }; // got: 5us, 13.4us, 103.2us, 1005us
};

void loop()
{

}








