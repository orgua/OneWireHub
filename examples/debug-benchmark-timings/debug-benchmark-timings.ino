/*
 *    Test-Code for a suitable timing function
 *
 *    - Var1: use micros()
 *    - Var2: count ticks
 */

#define TIMESLOT_WAIT_RETRY_COUNT       (microsecondsToClockCycles(1000))
#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))

void setup()
{
    Serial.begin(115200);
    Serial.println("Test-Code for a suitable timing function");

    uint32_t time_start, time_stop;

    /// timer test //////////////////////////////////////////////////
    uint16_t counter = 0;
    time_stop = micros() + 1000;
    while (micros() < time_stop)
    {
        counter++;
    }
    Serial.print("Benchmark: 1ms got us ");
    Serial.print(counter);
    Serial.println(" * (micros(), 32bit check, 32bit increment)");

    /// Benchmark for waitTimeSlot-Code //////////////////////////////

    uint8_t pin_bitMask = digitalPinToBitMask(8);
    volatile uint8_t *baseReg;
    baseReg = portInputRegister(digitalPinToPort(8));
    volatile uint8_t *reg asm("r30") = baseReg;
    DIRECT_WRITE_LOW(reg, pin_bitMask);
    DIRECT_MODE_OUTPUT(reg, pin_bitMask); // put it low, so it always reads zero
    const uint8_t value = 0;

    uint16_t retries = TIMESLOT_WAIT_RETRY_COUNT;
    Serial.print("1000ms translate to ");
    Serial.print(retries);
    Serial.println(" ticks.");
    Serial.flush();

    //////////// 16bit /////////////////////////////////////

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

    uint32_t retries32 = TIMESLOT_WAIT_RETRY_COUNT;
    time_start = micros();
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries32 == 0) break;
    }
    time_stop = micros();
    Serial.print("32bit Loop took ");
    Serial.print(time_stop - time_start);
    Serial.println(" us. ");

}

void loop()
{

}
