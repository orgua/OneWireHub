// taken with modifications from: https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.h
// Platform specific I/O definitions

#ifndef ONEWIREHUB_PLATFORM_H
#define ONEWIREHUB_PLATFORM_H

#if defined(ARDUINO) && (ARDUINO>=100)
#include <Arduino.h>
#endif

// determine gcc version, will produce number like 40803 for gcc 4.8.3
#if defined(__GNUC__)
#define ONEWIRE_GCC_VERSION ( (__GNUC__ * 10000) + (__GNUC_MINOR__ * 100) + __GNUC_PATCHLEVEL__)
#else
#define ONEWIRE_GCC_VERSION 0
#endif

#if defined(__AVR__) /* arduino (all with atmega, atiny) */

#define PIN_TO_BASEREG(pin)             (portInputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))
using io_reg_t = uint8_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL {13}; // instructions per loop, compare 0 takes 11, compare 1 takes 13 cycles

#elif defined(__MK20DX128__) || defined(__MK20DX256__) || defined(__MK66FX1M0__) || defined(__MK64FX512__) /* teensy 3.2 to 3.6 */
#define PIN_TO_BASEREG(pin)             (portOutputRegister(pin))
#define PIN_TO_BITMASK(pin)             (1)
#define DIRECT_READ(base, mask)         (*((base)+512))
#define DIRECT_MODE_INPUT(base, mask)   (*((base)+640) = 0)
#define DIRECT_MODE_OUTPUT(base, mask)  (*((base)+640) = 1)
#define DIRECT_WRITE_LOW(base, mask)    (*((base)+256) = 1)
#define DIRECT_WRITE_HIGH(base, mask)   (*((base)+128) = 1)
using io_reg_t = uint8_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL {8}; // instructions per loop

#elif defined(__MKL26Z64__) /* teensy LC */

#define PIN_TO_BASEREG(pin)             (portOutputRegister(pin))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_READ(base, mask)         ((*((base)+16) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   (*((base)+20) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  (*((base)+20) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    (*((base)+8) = (mask))
#define DIRECT_WRITE_HIGH(base, mask)   (*((base)+4) = (mask))
using io_reg_t = uint8_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL {0}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(__SAM3X8E__) || defined(__SAM3A8C__) || defined(__SAM3A4C__) /* arduino due */

#define PIN_TO_BASEREG(pin)             (&(digitalPinToPort(pin)->PIO_PER))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_READ(base, mask)         (((*((base)+15)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+5)) = (mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+4)) = (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+13)) = (mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+12)) = (mask))
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(address) (*(const uint8_t *)(address))
#endif
using io_reg_t = uint32_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL { 22 }; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(__PIC32MX__)

#define PIN_TO_BASEREG(pin)             (portModeRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_READ(base, mask)         (((*(base+4)) & (mask)) ? 1 : 0)  //PORTX + 0x10
#define DIRECT_MODE_INPUT(base, mask)   ((*(base+2)) = (mask))            //TRISXSET + 0x08
#define DIRECT_MODE_OUTPUT(base, mask)  ((*(base+1)) = (mask))            //TRISXCLR + 0x04
#define DIRECT_WRITE_LOW(base, mask)    ((*(base+8+1)) = (mask))          //LATXCLR  + 0x24
#define DIRECT_WRITE_HIGH(base, mask)   ((*(base+8+2)) = (mask))          //LATXSET + 0x28
using io_reg_t = uint32_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL { 0 }; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(ARDUINO_ARCH_ESP8266) /* nodeMCU, ESPduino, ... */

#define PIN_TO_BASEREG(pin)             ((volatile uint32_t*) GPO)
#define PIN_TO_BITMASK(pin)             (1 << pin)
#define DIRECT_READ(base, mask)         ((GPI & (mask)) ? 1 : 0)    //GPIO_IN_ADDRESS
#define DIRECT_MODE_INPUT(base, mask)   (GPE &= ~(mask))            //GPIO_ENABLE_W1TC_ADDRESS
#define DIRECT_MODE_OUTPUT(base, mask)  (GPE |= (mask))             //GPIO_ENABLE_W1TS_ADDRESS
#define DIRECT_WRITE_LOW(base, mask)    (GPOC = (mask))             //GPIO_OUT_W1TC_ADDRESS
#define DIRECT_WRITE_HIGH(base, mask)   (GPOS = (mask))             //GPIO_OUT_W1TS_ADDRESS
using io_reg_t = uint32_t; // define special datatype for register-access
// The ESP8266 has two possible CPU frequencies: 160 MHz (26 IPL) and 80 MHz (22 IPL) -> something influences the IPL-Value
constexpr uint8_t VALUE_IPL { (microsecondsToClockCycles(1) > 120) ? 26 : 22 }; // instructions per loop

#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32) /* ESP32 Family */

#define PIN_TO_BASEREG(pin)             (0)
#define PIN_TO_BITMASK(pin)             (pin)
#define DIRECT_READ(base, pin)          digitalRead(pin)
#define DIRECT_WRITE_LOW(base, pin)     digitalWrite(pin, LOW)
#define DIRECT_WRITE_HIGH(base, pin)    digitalWrite(pin, HIGH)
#define DIRECT_MODE_INPUT(base, pin)    pinMode(pin, INPUT)
#define DIRECT_MODE_OUTPUT(base, pin)   pinMode(pin,OUTPUT)
#define DELAY_MICROSECONDS(us)		    delayMicroseconds(us)
using io_reg_t = uint32_t; // define special data type for register-access
constexpr uint8_t VALUE_IPL { 39 }; // instructions per loop, for 40 and 80 MHz (see esp8266 difference)

#elif defined(ARDUINO_ARCH_SAMD) /* arduino family samd */
// arduino-zero is defined(__SAMD21G18A__)

// TODO: hack needed until alternative to IPL-approach is implemented
#define TMP_HACK
#ifdef TMP_HACK
#undef clockCyclesPerMicrosecond
#undef clockCyclesToMicroseconds
#undef microsecondsToClockCycles
// assuming SystemCoreClock is 48 MHz
#define MY_SYSCLK (48000000L)
//#define MY_SYSCLK (F_CPU)
#define clockCyclesPerMicrosecond() ( MY_SYSCLK / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (MY_SYSCLK / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (MY_SYSCLK / 1000000L) )
#endif

#define PIN_TO_BASEREG(pin)             portModeRegister(digitalPinToPort(pin))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define DIRECT_READ(base, mask)         (((*((base)+8)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) = (mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+2)) = (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+5)) = (mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+6)) = (mask))
using io_reg_t = uint32_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL { 18 }; // instructions per loop

#elif defined(NRF52) /* arduino primo */

#define PIN_TO_BASEREG(pin)             (0)
#define PIN_TO_BITMASK(pin)             (pin)
#define DIRECT_READ(base, pin)          nrf_gpio_pin_read(pin)
#define DIRECT_WRITE_LOW(base, pin)     nrf_gpio_pin_clear(pin)
#define DIRECT_WRITE_HIGH(base, pin)    nrf_gpio_pin_set(pin)
#define DIRECT_MODE_INPUT(base, pin)    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL)
#define DIRECT_MODE_OUTPUT(base, pin)   nrf_gpio_cfg_output(pin)
using io_reg_t = uint32_t; // define special data type for register-access
constexpr uint8_t VALUE_IPL {0}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(NRF51) /* red bear blend, should be good for all nrf51x chips */

#if defined(TARGET_NRF51822)
#include <nRF51822_API.h>
#endif

#define PIN_TO_BASEREG(pin)             (0)
#define PIN_TO_BITMASK(pin)             (pin)
#define DIRECT_READ(base, pin)          nrf_gpio_pin_read(pin)
#define DIRECT_WRITE_LOW(base, pin)     nrf_gpio_pin_clear(pin)
#define DIRECT_WRITE_HIGH(base, pin)    nrf_gpio_pin_set(pin)
#define DIRECT_MODE_INPUT(base, pin)    nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL)
#define DIRECT_MODE_OUTPUT(base, pin)   nrf_gpio_cfg_output(pin)
using io_reg_t = uint32_t; // define special data type for register-access
constexpr uint8_t VALUE_IPL {0}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(__RFduino__) /* rf51 chip with special implementation */

#define PIN_TO_BASEREG(pin)             (0)
#define PIN_TO_BITMASK(pin)             (pin)
#define DIRECT_READ(base, pin)          digitalRead(pin)
#define DIRECT_WRITE_LOW(base, pin)     digitalWrite(pin, LOW)
#define DIRECT_WRITE_HIGH(base, pin)    digitalWrite(pin, HIGH)
#define DIRECT_MODE_INPUT(base, pin)    pinMode(pin, INPUT)
#define DIRECT_MODE_OUTPUT(base, pin)   pinMode(pin,OUTPUT)
using io_reg_t = uint32_t; // define special data type for register-access
constexpr uint8_t VALUE_IPL {0}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#elif defined(__arc__) /* Arduino101/Genuino101 specifics */

#include "scss_registers.h"
#include "portable.h"
#include "avr/pgmspace.h"

#define GPIO_ID(pin)			(g_APinDescription[pin].ulGPIOId)
#define GPIO_TYPE(pin)			(g_APinDescription[pin].ulGPIOType)
#define GPIO_BASE(pin)			(g_APinDescription[pin].ulGPIOBase)
#define DIR_OFFSET_SS			0x01
#define DIR_OFFSET_SOC			0x04
#define EXT_PORT_OFFSET_SS		0x0A
#define EXT_PORT_OFFSET_SOC		0x50

/* GPIO registers base address */
#define PIN_TO_BASEREG(pin)		((volatile uint32_t *)g_APinDescription[pin].ulGPIOBase)
#define PIN_TO_BITMASK(pin)		pin
using io_reg_t = uint32_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL {0}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

static inline __attribute__((always_inline))
io_reg_t directRead(volatile io_reg_t *base, io_reg_t pin)
{
    io_reg_t ret;
    if (SS_GPIO == GPIO_TYPE(pin)) {
        ret = READ_ARC_REG(((io_reg_t)base + EXT_PORT_OFFSET_SS));
    } else {
        ret = MMIO_REG_VAL_FROM_BASE((io_reg_t)base, EXT_PORT_OFFSET_SOC);
    }
    return ((ret >> GPIO_ID(pin)) & 0x01);
}

static inline __attribute__((always_inline))
void directModeInput(volatile io_reg_t *base, io_reg_t pin)
{
    if (SS_GPIO == GPIO_TYPE(pin)) {
        WRITE_ARC_REG(READ_ARC_REG((((io_reg_t)base) + DIR_OFFSET_SS)) & ~(0x01 << GPIO_ID(pin)),
			((io_reg_t)(base) + DIR_OFFSET_SS));
    } else {
        MMIO_REG_VAL_FROM_BASE((io_reg_t)base, DIR_OFFSET_SOC) &= ~(0x01 << GPIO_ID(pin));
    }
}

static inline __attribute__((always_inline))
void directModeOutput(volatile io_reg_t *base, io_reg_t pin)
{
    if (SS_GPIO == GPIO_TYPE(pin)) {
        WRITE_ARC_REG(READ_ARC_REG(((io_reg_t)(base) + DIR_OFFSET_SS)) | (0x01 << GPIO_ID(pin)),
			((io_reg_t)(base) + DIR_OFFSET_SS));
    } else {
        MMIO_REG_VAL_FROM_BASE((io_reg_t)base, DIR_OFFSET_SOC) |= (0x01 << GPIO_ID(pin));
    }
}

static inline __attribute__((always_inline))
void directWriteLow(volatile io_reg_t *base, io_reg_t pin)
{
    if (SS_GPIO == GPIO_TYPE(pin)) {
        WRITE_ARC_REG(READ_ARC_REG(base) & ~(0x01 << GPIO_ID(pin)), base);
    } else {
        MMIO_REG_VAL(base) &= ~(0x01 << GPIO_ID(pin));
    }
}

static inline __attribute__((always_inline))
void directWriteHigh(volatile io_reg_t *base, io_reg_t pin)
{
    if (SS_GPIO == GPIO_TYPE(pin)) {
        WRITE_ARC_REG(READ_ARC_REG(base) | (0x01 << GPIO_ID(pin)), base);
    } else {
        MMIO_REG_VAL(base) |= (0x01 << GPIO_ID(pin));
    }
}

#define DIRECT_READ(base, pin)		    directRead(base, pin)
#define DIRECT_MODE_INPUT(base, pin)	directModeInput(base, pin)
#define DIRECT_MODE_OUTPUT(base, pin)	directModeOutput(base, pin)
#define DIRECT_WRITE_LOW(base, pin)	    directWriteLow(base, pin)
#define DIRECT_WRITE_HIGH(base, pin)	directWriteHigh(base, pin)

#else // any unknown architecture, including PC

#include <inttypes.h>

#define PIN_TO_BASEREG(pin)             (0)
#define PIN_TO_BITMASK(pin)             (pin)
#define DIRECT_READ(base, pin)          digitalRead(pin)
#define DIRECT_WRITE_LOW(base, pin)     digitalWrite(pin, LOW)
#define DIRECT_WRITE_HIGH(base, pin)    digitalWrite(pin, HIGH)
#define DIRECT_MODE_INPUT(base, pin)    pinMode(pin,INPUT)
#define DIRECT_MODE_OUTPUT(base, pin)   pinMode(pin,OUTPUT)
using io_reg_t = uint32_t; // define special datatype for register-access
constexpr uint8_t VALUE_IPL {10}; // instructions per loop, uncalibrated so far - see ./examples/debug/calibrate_by_bus_timing for an explanation

#warning "OneWire. Fallback mode. Using API calls for pinMode,digitalRead and digitalWrite. Operation of this library is not guaranteed on this architecture."

#endif



/////////////////////////////////////////// EXTRA PART /////////////////////////////////////////
// this part is loaded if no proper arduino-environment is found (good for external testing)
// these used functions are mockups
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ARDUINO
#define ONEWIREHUB_FALLBACK_BASIC_FNs
#define ONEWIREHUB_FALLBACK_ADDITIONAL_FNs // to load up Serial below
#endif

#ifdef ARDUINO_attiny
#define ONEWIREHUB_FALLBACK_ADDITIONAL_FNs // to load up Serial below
#endif

#ifdef ONEWIREHUB_FALLBACK_BASIC_FNs

#define INPUT 1
#define INPUT_PULLUP 1
#define OUTPUT 0
#define HIGH 1
#define LOW 0


static bool mockup_pin_value[256];

template<typename T1>
bool digitalRead(const T1 pin) { return (mockup_pin_value[pin & 0xFF] != 0); }; // mock up outputs

template<typename T1, typename T2>
void digitalWrite(const T1 pin, const T2 value) { mockup_pin_value[pin & 0xFF] = value; };

template<typename T1, typename T2>
void pinMode(const T1 pin, const T2 value) { mockup_pin_value[pin & 0xFF] = value; };

template<typename T1>
T1 digitalPinToPort(const T1 pin) { return pin; };

template<typename T1>
T1 * portInputRegister(const T1 port) { return port; };

template<typename T1>
T1 digitalPinToBitMask(const T1 pin) { return pin; };

constexpr uint32_t microsecondsToClockCycles(const uint32_t micros) { return (100*micros); }; // mockup, emulate 100 MHz CPU

template<typename T1>
void delayMicroseconds(const T1 micros) { };

/// the following fn are no templates and need to be defined in platform.cpp

uint32_t micros(); // takes about 3 µs to process @ 16 MHz

void cli();
void sei();

void noInterrupts();

void interrupts();

template<typename T1>
T1 pgm_read_byte(const T1* address)
{
    return *address;
}

#endif


#ifdef ONEWIREHUB_FALLBACK_ADDITIONAL_FNs // Test to make it work on aTtiny85, 8MHz
/// README: use pin2 or pin3 for Attiny, source: https://github.com/gioblu/PJON/wiki/ATtiny-interfacing

#ifndef BIN
#define BIN 1
#endif

#ifndef HEX
#define HEX 2
#endif

static class serial
{
private:

    uint32_t speed;

public:

    void print(...) { };

    void println(...) { };

    void flush() { };
    void begin(const uint32_t speed_baud) { speed = speed_baud; };

} Serial;


template<typename T1, typename T2>
void memset(T1 * const address, const T1 initValue, const T2 bytes)
{
    const T2 iterations = bytes / sizeof(T1);
    for (T2 counter = 0; counter < iterations; ++counter)
    {
        address[counter] = (initValue);
    }
}


template<typename T1, typename T2>
void memcpy(T1 * const destination, const T1* const source, const T2 bytes)
{
    const T2 iterations = bytes / sizeof(T1);
    for (T2 counter = 0; counter < iterations; ++counter)
    {
        destination[counter] = source[counter];
    }
}


template<typename T1, typename T2>
bool memcmp(const T1* const source_A, const T1* const source_B, const T2 bytes) // return true if string is different
{
    const T2 iterations = bytes / sizeof(T1);
    for (T2 counter = 0; counter < iterations; ++counter)
    {
        if (source_A[counter] != source_B[counter]) return true;
    };
    return false;
}

void        delay(uint32_t time_millis);
uint32_t    millis(void);

void        wdt_reset(void);
void        wdt_enable(...);

#ifndef PROGMEM
#define PROGMEM
#endif // PROGMEM

#endif // ONEWIREHUB_FALLBACK_ADDITIONAL_FNs

#endif //ONEWIREHUB_PLATFORM_H
