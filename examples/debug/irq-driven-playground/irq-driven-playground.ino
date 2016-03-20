/*
 *    Playground for new IRQ - concept
 */

#include <setjmp.h>
static jmp_buf break_here;

// setjmp(env) safes the current state in the program and returns 0
// longjmp(env) goes back to that state saved state --> setjmp will exit with 1

/*   DOES NOT WORK
 *
 *   Concept for the OneWireHub:
 *   - structure of current Code could be left intact, except FN with DirectPinModifications:
 *      - revcBit(), sendBit, waitWhilePinIs()
 *   - ISR fires on PIN-Change and
 *      - first action would be:
 *         if(StaticVarIsOne) {
 *            StaticVarIsOne = 0;
 *            if (setjmp(fallback)) return; //safe ONE time
 *         };
 *      - takes a timestamp if needed (reset detection, wait for next timeslot, detect zero/one)
 *      - waits a specific time to send a zero or show presence
 *   - setjump would be called instead of classical waiting
 *      - exception: the wait-scenarios named above
 *      - after (!setjump(env)) the routine would longjump(fallback,1)
 *   - there should be a fn() that says "sleep is allowed"
 *      - short: YES if waiting for falling edge, NO if waiting for rising edge
 *      - it is ok while waiting for the next bit to send or receive
 *      - it is NOT ok to sleep when detecting a reset (timer would be unavail during sleep)
 *   -
 */


const uint8_t led_PIN       = 13;

bool blinking()
{
    const  uint32_t interval    = 500;          // interval at which to blink (milliseconds)
    static uint32_t nextMillis  = millis();     // will store next time LED will updated

    if (millis() > nextMillis)
    {
        nextMillis += interval;             // save the next time you blinked the LED
        static uint8_t ledState = LOW;      // ledState used to set the LED
        if (ledState == LOW)    ledState = HIGH;
        else                    ledState = LOW;
        digitalWrite(led_PIN, ledState);
        return 1;
    }
    return 0;
}


volatile uint8_t has_to_resume = 0;

void irq_mockup()
{
    static uint8_t dataByte = 0;

    if (has_to_resume)
    {
        has_to_resume = 0;
        longjmp(break_here,1);
    }

    //// put the state-machine here
    // waitReset
    // showPresence
/*
    static bool flipFlop = true;

    if (flipFlop)
    {
        //flipFlop = false;
        if (!send(dataByte++)) return;
    }
    else
    {
        flipFlop = true;
        if (!recv()) return;
    }
*/
    if (!send(dataByte++)) return;

    //Serial.println(" IRQ End");
}

bool send(const uint8_t dataByte)
{
    Serial.print(" Sending 0x");
    Serial.print(dataByte,HEX);
    Serial.print(": ");

    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (!sendBit((bitMask & dataByte) ? 1 : 0)) return false;
    }
    has_to_resume = 0;

    Serial.println("done");
    return true;
}



bool sendBit(const bool value)
{
    Serial.print(value);
    Serial.print(" ");
    Serial.flush();
    // wait for next Timeslot
    if (setjmp(break_here))
    {
        return true;
    }
    else
    {
        has_to_resume = 1;
        return false; // no bit send yet
    }
}
/*
bool recv(void)
{
    Serial.print(" Receiving:");
    static uint8_t fake_recv = 0;

    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (!recvBit(fake_recv--&bitMask))  return false;
    }
    has_to_resume = 0;

    Serial.println(" done");
    return true;
}

bool recvBit(uint8_t fake_recv)
{

    // wait for next Timeslot
    if (setjmp(break_here))
    {
        if (fake_recv) Serial.print("1 ");
        else Serial.print("0 ");

        return true;
    }
    else
    {
        has_to_resume = 1;
        return false; // no bit recv yet
    }
}
*/

void setup()
{
    Serial.begin(115200);
    Serial.println("Start of Setup.");
    pinMode(led_PIN, OUTPUT);
}

void loop()
{
    // Blink triggers the state-change
    if (blinking())
    {
        irq_mockup(); // emulate an interrupt
        //Serial.println(" jump end ");
    }
}