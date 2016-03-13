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

void setup()
{
    Serial.begin(115200);
    Serial.println("Test-Code for a suitable timing function");
    Serial.flush();

    // setup direct pin-access
    uint8_t pin_bitMask = digitalPinToBitMask(8);
    volatile uint8_t *baseReg;
    baseReg = portInputRegister(digitalPinToPort(8));
    volatile uint8_t *reg asm("r30") = baseReg;
    uint8_t value = 0;

    /// Benchmark for waitTimeSlot-Code //////////////////////////////

    DIRECT_WRITE_LOW(reg, pin_bitMask);
    DIRECT_MODE_OUTPUT(reg, pin_bitMask); // put it low, so it always reads zero

    Serial.print("1000ms translate to ");
    Serial.print(microsecondsToClockCycles(1000));
    Serial.println(" ticks.");
    Serial.flush();

    /// Benchmark for a timer test //////////////////////////////////////////////////

    {
        uint16_t counter = 0;
        uint32_t time_stop = micros() + 1000;
        while (micros() < time_stop)
        {
            counter++;
        }
        Serial.print("Benchmark: 1ms got us ");
        Serial.print(counter);
        Serial.println(" * (micros(), 32bit check, 16bit increment)");
        Serial.flush();
    }

    //////////// 16bit /////////////////////////////////////

    {
        uint16_t retries = microsecondsToClockCycles(1000);
        uint32_t time_start = micros();
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }
        uint32_t time_stop = micros();

        Serial.print("16bit Loop took ");
        Serial.print(time_stop - time_start);
        Serial.println(" us. ");
        Serial.flush();
    }

    //////////// 32bit /////////////////////////////////////

    uint32_t time;
    {
        uint32_t retries = microsecondsToClockCycles(1000);
        uint32_t time_start = micros();
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }
        uint32_t time_stop = micros();
        time = time_stop - time_start;

        Serial.print("32bit Loop took ");
        Serial.print(time);
        Serial.println(" us. ");
        Serial.flush();
    }

    //////////// 32bit, 1ms /////////////////////////////////////

    {
        uint32_t retries  = microsecondsToClockCycles(1000) * 1000 / time;

        uint32_t time_start = micros();
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }
        uint32_t time_stop = micros();

        Serial.print("32bit 1ms-Loop took ");
        Serial.print(time_stop - time_start);
        Serial.println(" us. ");
        Serial.flush();
    }

    //////////// test timings on the Pin /////////////////////////////////////
    // use logic probe or osci to confirm programming
    // 1.0ms: High, then low
    // 0.1ms: High, then Low
    {
        uint16_t retries;

        DIRECT_WRITE_HIGH(reg, pin_bitMask);
        value = 1;
        retries = microsecondsToClockCycles(1000);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }

        DIRECT_WRITE_LOW(reg, pin_bitMask);
        value = 0;
        retries = microsecondsToClockCycles(1000);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }

        DIRECT_WRITE_HIGH(reg, pin_bitMask);
        value = 1;
        retries = microsecondsToClockCycles(100);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }

        DIRECT_WRITE_LOW(reg, pin_bitMask);
        value = 0;
        retries = microsecondsToClockCycles(100);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }
    }

    //////////// try to exit the loop early on success //////////////////////////////

    {
        DIRECT_WRITE_HIGH(reg, pin_bitMask);
        value = 0;
        uint16_t retries = microsecondsToClockCycles(10);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }

        DIRECT_WRITE_LOW(reg, pin_bitMask);
        value = 1;
        retries = microsecondsToClockCycles(10);
        while (DIRECT_READ(reg, pin_bitMask) == value)
        {
            if (--retries == 0) break;
        }

        // toggle pin one last time to time the last slot
        DIRECT_WRITE_HIGH(reg, pin_bitMask);
    }

}

void loop()
{

}
