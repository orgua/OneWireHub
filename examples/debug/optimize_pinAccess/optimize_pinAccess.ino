/*
 *    Test-Code for portable hardware access
 *
 *   atmega328@16MHz makes around 571 kHz with pin-toggling
 */

#include "OneWireHub.h"

/////////////////////////////////////////////////////////////////////////
/////// From OnewireHub <0.9.7 //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

uint8_t pin_bitMaskL;
volatile uint8_t *baseRegL;

void pinConfigLegacy(const uint8_t pin)
{
    // setup direct pin-access
    pin_bitMaskL = digitalPinToBitMask(pin);
    baseRegL = portInputRegister(digitalPinToPort(pin));
}

void pinTestLegacy(void)
{
    volatile uint8_t *reg asm("r30") = baseRegL; // note: asm only for AVR, really needed? investigate

    DIRECT_WRITE_LOW(reg, pin_bitMaskL);
    DIRECT_MODE_OUTPUT(reg, pin_bitMaskL); // set it low, so it always reads zero

    while(1)
    {
        DIRECT_WRITE_HIGH(reg, pin_bitMaskL);
        DIRECT_WRITE_LOW(reg, pin_bitMaskL);
    }
}


/////////////////////////////////////////////////////////////////////////
/////// From OnewireLib 2.3.2  //////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

/*
#define PIN_TO_BASEREG(pin)             (portInputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define IO_REG_TYPE uint8_t
#define IO_REG_ASM asm("r30")
*/

IO_REG_TYPE bitmask;
volatile IO_REG_TYPE *baseReg;

void pinConfigOneWireLib(const uint8_t pin)
{
    pinMode(pin, OUTPUT); // why output? not ok for 1Wire
    bitmask = PIN_TO_BITMASK(pin);
    baseReg = PIN_TO_BASEREG(pin);
}


void pinTestOneWireLib(void)
{
    IO_REG_TYPE mask = bitmask; // note: why? it is already done, why not const
    volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg; // TODO: really needed as a copy? why volatile

    // call
    noInterrupts(); // note: why needed? it does not need to be atomic, only with pin-changing interrupts
    DIRECT_WRITE_LOW(reg, mask);
    DIRECT_MODE_OUTPUT(reg, mask);	// drive output low
    interrupts();

    while(1)
    {
        DIRECT_WRITE_HIGH(reg, mask);
        DIRECT_WRITE_LOW(reg, mask);
    }
}

////////////////////////////////////////////////////////////////////////////////

using io_reg_t = uint8_t; // define special datatype for register-access

io_reg_t pin_bitMask;
volatile io_reg_t *pin_baseReg; // needs to be volatile, because its only written but never read, so it gets optimized out

void pinConfigClean(const uint8_t pin)
{
    pinMode(pin, INPUT); // as a OW-slave we should mostly listen
    // setup direct pin-access
    pin_bitMask = PIN_TO_BITMASK(pin);
    pin_baseReg = PIN_TO_BASEREG(pin);
}

void pinTestClean(void)
{
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask); // put it low, so it always reads zero

    while(1)
    {
        DIRECT_WRITE_HIGH(pin_baseReg, pin_bitMask);
        DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    }
}


void setup()
{
    const uint8_t pin_test = 8;

    // measurement with oszi --> each of this work with an atmega328p, 16MHz Clock bring 571 kHz pinFreq for case 1-3, double for case 4
    switch(4)
    {
        case 0:
        case 1:
            pinConfigLegacy(pin_test);
            pinTestLegacy();
            break;

        case 2:
            pinConfigOneWireLib(pin_test);
            pinTestOneWireLib();
            break;

        case 3:
            pinConfigClean(pin_test);
            pinTestClean();
            break;

        case 4: // brings 1310 kHz
            pinConfigClean(pin_test);
            noInterrupts();
            pinTestClean();
            break;
    }
}

void loop()
{

}
