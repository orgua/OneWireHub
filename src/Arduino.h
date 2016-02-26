#ifndef ONEWIREHUB_ARDUINO_H
#define ONEWIREHUB_ARDUINO_H

#if  defined(ARDUINO)
#include <Arduino.h>
#else

// this part is loaded if no proper arduino-environment is found (good for external testing)
// these functions are mockups and used by the Hub

#include "inttypes.h"

#define HEX 1
#define INPUT 1
#define OUTPUT 0
#define HIGH 1
#define LOW 0

class serial
{
public:
    template <typename T1>
    void print(T1 x);

    template <typename T1, typename T2>
    void print(T1 x,T2 y);

    template <typename T1>
    void println(T1 x);

    template <typename T1, typename T2>
    void println(T1 x,T2 y);

    void print(void);
    void println(void);
    void flush(void);
    void begin(uint32_t x);

};
serial Serial;

template <typename T1>
uint8_t digitalRead(T1);

template <typename T1, typename T2>
uint8_t digitalWrite(T1, T2);

template <typename T1, typename T2>
uint8_t pinMode(T1, T2);

uint8_t digitalPinToPort(uint8_t x);
uint8_t *portInputRegister(uint8_t x);
uint8_t digitalPinToBitMask(uint8_t x);

uint32_t microsecondsToClockCycles(uint32_t x);

uint32_t micros(void); // takes about 3 µs to process @ 16 MHz
//void delayMicroseconds(uint32_t x);

void cli(void);
void sei(void);

template <typename T1, typename T2, typename T3>
void memset(T1 x, T2 y, T3 z);

#endif
#endif //ONEWIREHUB_ARDUINO_H
