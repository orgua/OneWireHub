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

#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))

void setup()
{
    Serial.begin(115200);
    Serial.println("Test-Code for a suitable timing function");

    // setup timers
    uint32_t time_start, time_stop;

    // setup direct pin-access
    uint8_t pin_bitMask = digitalPinToBitMask(8);
    volatile uint8_t *baseReg;
    baseReg = portInputRegister(digitalPinToPort(8));
    volatile uint8_t *reg asm("r30") = baseReg;
    uint8_t value = 0;

    /// Benchmark for a timer test //////////////////////////////////////////////////

    uint16_t counter = 0;
    time_stop = micros() + 1000;
    while (micros() < time_stop)
    {
        counter++;
    }
    Serial.print("Benchmark: 1ms got us ");
    Serial.print(counter);
    Serial.println(" * (micros(), 32bit check, 32bit increment)");
    Serial.flush();

    /// Benchmark for waitTimeSlot-Code //////////////////////////////

    DIRECT_WRITE_LOW(reg, pin_bitMask);
    DIRECT_MODE_OUTPUT(reg, pin_bitMask); // put it low, so it always reads zero

    uint16_t retries = microsecondsToClockCycles(1000);
    Serial.print("1000ms translate to ");
    Serial.print(retries);
    Serial.println(" ticks.");
    Serial.flush();

    //////////// 16bit /////////////////////////////////////

    retries = microsecondsToClockCycles(1000);
    time_start = micros();
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
            if (--retries == 0) break;
    }
    time_stop = micros();

    Serial.print("16bit Loop took ");
    Serial.print(time_stop - time_start);
    Serial.println(" us. ");
    Serial.flush();

    //////////// 32bit /////////////////////////////////////

    uint32_t retries32 = microsecondsToClockCycles(1000);
    time_start = micros();
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries32 == 0) break;
    }
    time_stop = micros();

    Serial.print("32bit Loop took ");
    Serial.print(time_stop - time_start);
    Serial.println(" us. ");
    Serial.flush();

    //////////// test timings on the Pin /////////////////////////////////////
    // use logic probe or osci to confirm programming
    // 1.0ms: High, then low
    // 0.1ms: High, then Low

    DIRECT_WRITE_HIGH(reg, pin_bitMask);
    value   = 1;
    retries = microsecondsToClockCycles(1000);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    DIRECT_WRITE_LOW(reg, pin_bitMask);
    value   = 0;
    retries = microsecondsToClockCycles(1000);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    DIRECT_WRITE_HIGH(reg, pin_bitMask);
    value   = 1;
    retries = microsecondsToClockCycles(100);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    DIRECT_WRITE_LOW(reg, pin_bitMask);
    value   = 0;
    retries = microsecondsToClockCycles(100);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    //////////// try to exit the loop early on success //////////////////////////////

    DIRECT_WRITE_HIGH(reg, pin_bitMask);
    value   = 0;
    retries = microsecondsToClockCycles(10);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    DIRECT_WRITE_LOW(reg, pin_bitMask);
    value   = 1;
    retries = microsecondsToClockCycles(10);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) break;
    }

    // toggle pin one last time to time the last slot
    DIRECT_WRITE_HIGH(reg, pin_bitMask);

}

void loop()
{

}
