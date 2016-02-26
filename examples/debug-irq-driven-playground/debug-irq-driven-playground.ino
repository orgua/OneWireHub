/*
 *    Playground for new IRQ - concepts
 */

#include <setjmp.h>
jmp_buf env;

// setjmp(env) safes the current state in the program and returns 0
// longjmp(env) goes back to that state if called

/*
 *   Concept for the OneWireHub:
 *   - structure of current Code could be left intact, except FN with DirectPinModifications:
 *      - revcBit(), sendBit, waitWhilePinIs()
 *   - ISR fires on PIN-Change and
 *      - first action would be:
 *         if(StaticVarIsOne) {
 *            StaticVarIsOne = 0;
 *            if (setjump(fallback)) return; //safe ONE time
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


uint8_t countGe = 0; // will preserve its state

void setup()
{

    static uint8_t countSe = 0; // will preserve the state
    uint8_t countMe = 0; // will be restored to 0 after jump

    Serial.begin(115200);
    Serial.println("Start of Setup.");

    if (setjmp (env))
    {
        Serial.println("jumped where no man jumped before");
        delay(1000);
    }

    Serial.print("End of Setup S");
    Serial.print(countSe++);
    Serial.print(" M");
    Serial.println(countMe++);
}

void loop()
{

    static uint8_t countLe = 0; // will be also unaffected by jump

    // Blink triggers the state-change
    if (blinking())
    {
        Serial.print("right before jump G");
        Serial.print(countGe++);
        Serial.print(" L");
        Serial.println(countLe++);
        Serial.flush();
        longjmp (env,1);
        Serial.println("right after jump");
    }
}