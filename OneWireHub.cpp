#include "OneWireHub.h"
#include "pins_arduino.h"
#include <Arduino.h>

extern "C" {
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
}

#define DIRECT_READ(base, mask)        (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)  ((*(base+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask) ((*(base+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)   ((*(base+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)  ((*(base+2)) |= (mask))

#define TIMESLOT_WAIT_RETRY_COUNT microsecondsToClockCycles(120) / 10L
#define TIMESLOT_WAIT_READ_RETRY_COUNT microsecondsToClockCycles(135)

//--- CRC 16 ---
static uint16_t crc16;

void ow_crc16_reset(void)
{
    crc16 = 0;
}

void ow_crc16_update(uint8_t b)
{
    for (uint8_t j = 0; j < 8; j++)
    {
        uint8_t mix = ((uint8_t) crc16 ^ b) & 0x01;
        crc16 = crc16 >> 1;
        if (mix)
            crc16 = crc16 ^ 0xA001;

        b = b >> 1;
    }
}

uint16_t ow_crc16_get(void)
{
    return crc16;
}

//=================== Hub ==========================================
OneWireHub::OneWireHub(uint8_t pin)
{
    pin_bitmask = digitalPinToBitMask(pin);
    slave_count = 0;
    baseReg = portInputRegister(digitalPinToPort(pin));

    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; i++)
        this->elms[i] = nullptr;
}

bool OneWireHub::waitForRequest(bool ignore_errors)
{
    errno = ONEWIRE_NO_ERROR;

    for (; ;)
    {
        //delayMicroseconds(40);
        //Once reset is done, it waits another 30 micros
        //Master wait is 65, so we have 35 more to send our presence now that reset is done
        if (!waitReset(0))
        {
            continue;
        }

        //Reset is complete, tell the master we are prsent
        // This will pull the line low for 125 micros (155 micros since the reset) and
        //  then wait another 275 plus whatever wait for the line to go high to a max of 480
        // This has been modified from original to wait for the line to go high to a max of 480.
        if (!presence())
        {
            continue;
        }

        //Now that the master should know we are here, we will get a command from the line
        //Because of our changes to the presence code, the line should be guranteed to be high
        if (recvAndProcessCmd())
        {
            return TRUE;
        }
        else if ((errno == ONEWIRE_NO_ERROR) || ignore_errors)
        {
            continue;
        }
        else
        {
            return FALSE;
        }
    }
}

// attach a sensor to the hub
uint8_t OneWireHub::attach(OneWireItem &sensor)
{
    if (slave_count >= ONEWIRESLAVE_COUNT) return 0; // hub is full

    // find position of next free storage-position
    uint8_t position = 0;
    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; i++)
    {
        if (this->elms[i] == nullptr)
        {
            position = i;
            break;
        }
    }
    // TODO: should we also look for already attached sensors?

    elms[position] = &sensor;
    slave_count++;
    calck_mask();
    return position;
};

bool    OneWireHub::detach(const OneWireItem &sensor)
{
    // find position of sensor
    uint8_t position = 255;
    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; i++)
    {
        if (this->elms[i] == &sensor)
        {
            position = i;
            break;
        }
    }

    if (position != 255)
        return detach(position);
    else
        return 0;
};

bool    OneWireHub::detach(const uint8_t slave_number)
{
    if (elms[slave_number] == nullptr)
        return 0;
    if (!slave_count)
        return 0;

    elms[slave_number] == nullptr;
    slave_count--;
    calck_mask();
    return 1;
};


int OneWireHub::calck_mask(void) // TODO: is CALCK is typo?
{

    if (dbg_CALCK)
    {
        Serial.print("Time: ");
        Serial.println(micros());
    }

    uint8_t Pos = 0;

    // Zerro
    for (int i = 0; i < ONEWIREIDMAP_COUNT; i++)
    {
        this->bits[i] = 3;
        this->idmap0[i] = 0;
        this->idmap1[i] = 0;
    }

    // Get elms mask
    uint8_t mask = 0x00;
    for (int i = 0; i < ONEWIRESLAVE_COUNT; i++)
    {
        if (this->elms[i] == nullptr) continue;
        mask = mask | (1 << i);
    }

    if (dbg_CALCK)
    {
        Serial.print("Mask:");
        Serial.println(mask, HEX);
    }

    // First data
    uint8_t stack[8][5]; // operate bit, set pos, Byte pos, byte mask, elms mask

    // 0
    stack[0][0] = 0;    // bit
    stack[0][1] = 0xFF; // None
    stack[0][2] = 0x00; // Pos
    stack[0][3] = 0x01; // Mask
    stack[0][4] = mask; // Elms mask
    uint8_t stackpos = 1;

    while (stackpos)
    {
        if (Pos >= ONEWIREIDMAP_COUNT) return 0;

        if (dbg_CALCK)
        {
            Serial.print("Pos=");
            Serial.print(Pos);
            Serial.print("\t");

            Serial.print("SLevel=");
            Serial.print(stackpos);
            Serial.print("\t");
        }

        stackpos--;

        // Set last step jamp
        uint8_t spos = stack[stackpos][1];
        uint8_t BN = stack[stackpos][2];
        uint8_t BM = stack[stackpos][3];
        uint8_t mask = stack[stackpos][4];

        if (spos != 0xFF)
        {

            if (stack[stackpos][0])
            {
                if (dbg_CALCK)
                {
                    Serial.print("OPos:1=");
                    Serial.print(spos);
                    Serial.print("->");
                    Serial.print(Pos);
                }

                this->idmap1[spos] = Pos;
            }
            else
            {
                if (dbg_CALCK)
                {
                    Serial.print("OPos:0=");
                    Serial.print(spos);
                    Serial.print("->");
                    Serial.print(Pos);
                }

                this->idmap0[spos] = Pos;
            }
        }
        else if (dbg_CALCK) Serial.print("OPos:None");

        if (dbg_CALCK)
        {
            Serial.print("\t");
            Serial.print("BN=");
            Serial.print(BN);
            Serial.print("\t");

            Serial.print("BM=");
            Serial.print(BM, HEX);
            Serial.print("\t");
        }

        // Div tree
        bool fl0 = FALSE;
        bool fl1 = FALSE;
        uint8_t mask1 = 0x00;
        uint8_t mask0 = 0x00;
        uint8_t elmmask = 0x01;

        for (int i = 0; i < ONEWIRESLAVE_COUNT; i++)
        {
            if (elmmask & mask)
            {
                if (BM & this->elms[i]->ID[BN])
                {
                    mask1 = mask1 | elmmask;
                    fl1 = TRUE;
                } else
                {
                    mask0 = mask0 | elmmask;
                    fl0 = TRUE;
                }
            }

            elmmask = elmmask << 1;
        }

        if ((fl0 == FALSE) && (fl1 == TRUE))
        {
            this->bits[Pos] = 1;
        } else
        {
            if ((fl0 == TRUE) && (fl1 == FALSE))
            {
                this->bits[Pos] = 2;
            } else
            {
                this->bits[Pos] = 0;
            }
        }

        if (dbg_CALCK)
        {
            Serial.print("\t");
            Serial.print("Bit=");
            Serial.print(this->bits[Pos]);
            Serial.print("\t");

            Serial.print("mask0=");
            Serial.print(mask0, HEX);
            Serial.print("\t");

            Serial.print("mask1=");
            Serial.print(mask1, HEX);
            Serial.print("\t");
        }

        uint8_t NBN = BN;
        uint8_t NBM = BM << 1;
        if (!NBM)
        {
            NBN++;
            NBM = 0x01;

            // END
            if (NBN >= 8)
            {
                this->idmap0[Pos] = mask0;
                this->idmap1[Pos] = mask1;

                Pos++;
                if (dbg_CALCK) Serial.println();
                continue;
            }
        }

        // Tree 0
        if (mask0 != 0)
        {
            stack[stackpos][0] = 0;
            stack[stackpos][1] = Pos;
            stack[stackpos][2] = NBN;
            stack[stackpos][3] = NBM;
            stack[stackpos][4] = mask0;
            stackpos++;

            if (dbg_CALCK) Serial.print("ADD=0\t");
        }

        // Tree 1
        if (mask1 != 0)
        {
            stack[stackpos][0] = 1;
            stack[stackpos][1] = Pos;
            stack[stackpos][2] = NBN;
            stack[stackpos][3] = NBM;
            stack[stackpos][4] = mask1;
            stackpos++;

            if (dbg_CALCK) Serial.print("ADD=1\t");
        }

        if (dbg_CALCK) Serial.println();
        Pos++;
    }

    if (dbg_CALCK)
    {
        Serial.print("Time: ");
        Serial.println(micros());

        for (int i = 0; i < ONEWIREIDMAP_COUNT; i++)
        {
            Serial.print(i);
            Serial.print("\t");
            Serial.print(this->bits[i]);
            Serial.print("\t");
            Serial.print(this->idmap0[i]);
            Serial.print("\t");
            Serial.print(this->idmap1[i]);
            Serial.println();
        }
    }

    return Pos;
}

bool OneWireHub::waitReset(uint16_t timeout_ms)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    unsigned long time_stamp;

    errno = ONEWIRE_NO_ERROR;
    cli();
    DIRECT_MODE_INPUT(reg, mask);
    sei();

    //Wait for the line to fall
    if (timeout_ms != 0)
    {
        time_stamp = micros() + timeout_ms * 1000;
        while (DIRECT_READ(reg, mask))
        {
            if (micros() > time_stamp)
            {
                errno = ONEWIRE_WAIT_RESET_TIMEOUT;
                return FALSE;
            }
        }
    } else
    {
        //Will wait forever for the line to fall
        while (DIRECT_READ(reg, mask))
        { };
    }

    //Set to wait for rise up to 540 micros
    //Master code sets the line low for 500 micros
    //TODO The actual documented max is 640, not 540
    time_stamp = micros() + 540;

    //Wait for the rise on the line up to 540 micros
    while (DIRECT_READ(reg, mask) == 0)
    {
        if (micros() > time_stamp)
        {
            errno = ONEWIRE_VERY_LONG_RESET;
            return FALSE;
        }
    }

    //If the master pulled low for exactly 500, then this will be 40 wait time
    // Recommended for master is 480, which would be 60 here then
    // Max is 640, which makes this negative, but it returns above as a "ONEWIRE_VERY_LONG_RESET"
    // this gives an extra 10 to 30 micros befor calling the reset invalid
    if ((time_stamp - micros()) > 70)
    {
        errno = ONEWIRE_VERY_SHORT_RESET;
        return FALSE;
    }

    //Master will now delay for 65 to 70 recommended or max of 75 before it's "presence" check
    // and then read the pin value (checking for a presence on the line)
    // then wait another 490 (so, 500 + 64 + 490 = 1054 total without consideration of actual op time) on Arduino,
    // but recommended is 410 with total reset length of 480 + 70 + 410 (or 480x2=960)
    delayMicroseconds(30);
    //Master wait is 65, so we have 35 more to send our presence now that reset is done
    return TRUE;
}

bool OneWireHub::waitReset(void)
{
    return waitReset(1000);
}

bool OneWireHub::presence(uint8_t delta)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;

    //Reset code already waited 30 prior to calling this
    // Master will not read until 70 recommended, but could read as early as 60
    // so we should be well enough ahead of that. Arduino waits 65
    errno = ONEWIRE_NO_ERROR;
    cli();
    DIRECT_WRITE_LOW(reg, mask);
    DIRECT_MODE_OUTPUT(reg, mask);    // drive output low
    sei();

    //Delaying for another 125 (orignal was 120) with the line set low is a total of at least 155 micros
    // total since reset high depends on commands done prior, is technically a little longer
    delayMicroseconds(125);
    cli();
    DIRECT_MODE_INPUT(reg, mask);     // allow it to float
    sei();

    //Default "delta" is 25, so this is 275 in that condition, totaling to 155+275=430 since the reset rise
    // docs call for a total of 480 possible from start of rise before reset timing is completed
    //This gives us 50 micros to play with, but being early is probably best for timing on read later
    //delayMicroseconds(300 - delta);
    delayMicroseconds(250 - delta);

    //Modified to wait a while (roughly 50 micros) for the line to go high
    // since the above wait is about 430 micros, this makes this 480 closer
    // to the 480 standard spec and the 490 used on the Arduino master code
    // anything longer then is most likely something going wrong.
    uint8_t retries = 25;
    while (!DIRECT_READ(reg, mask));
    do
    {
        if (retries-- == 0)
            return FALSE;
        delayMicroseconds(2);
    } while (!DIRECT_READ(reg, mask));

//    if ( !DIRECT_READ(reg, mask)) {
//        errno = ONEWIRE_PRESENCE_LOW_ON_LINE;
//        return FALSE;
//    } else
//        return TRUE;
}

bool OneWireHub::presence(void)
{
    return presence(25);
}

bool OneWireHub::search(void)
{
    uint8_t bitmask;
    uint8_t bit_recv;
//  bool bit_n;

    int j;
    int flag;

    //Serial.println("--");
    //this->SelectElm = 0;

    int n = 0;
    for (int i = 0; i < 8; i++)
    {
        for (bitmask = 0x01; bitmask; bitmask <<= 1)
        {

            // Get from elements
            switch (this->bits[n])
            {
                case 0:
                    sendBit(FALSE);
                    sendBit(FALSE);
                    break;
                case 1:
                    sendBit(TRUE);
                    sendBit(FALSE);
                    break;
                case 2:
                    sendBit(FALSE);
                    sendBit(TRUE);
                    break;
                default:
                    return FALSE;
            }

/*
      bit_n = this->bits[n];
      sendBit( bit_n && 0x01 );
      sendBit( bit_n && 0x02 );
*/

            bit_recv = recvBit();

            if (errno != ONEWIRE_NO_ERROR) return FALSE;

            // Get next elm
            if (bit_recv)  n = this->idmap1[n]; // got a 1
            else           n = this->idmap0[n]; // got a 0

            // Test not found
            if (n == 0)
            {
                if (dbg_SEARCH)
                {
                    Serial.print("Not found-");
                    Serial.print(i);
                    Serial.print(",");
                    Serial.println(bitmask, HEX);
                }
                return FALSE;
            }
        }
    }

    for (int i = 0; i < ONEWIRESLAVE_COUNT; i++)
        if (i == (1 << i)) this->SelectElm = elms[i];

    if (dbg_SEARCH)
    {
        Serial.print("Found-");
        Serial.println(n);
    }

    return TRUE;
}

bool OneWireHub::recvAndProcessCmd(void)
{
    uint8_t addr[8];
    bool flag;

    for (; ;)
    {
        uint8_t cmd = recv();

        switch (cmd)
        {
            // Search rom
            case 0xF0:
                search();
                delayMicroseconds(6900);
                return FALSE;

                // MATCH ROM - Choose/Select ROM
            case 0x55:
                recvData(addr, 8);
                if (errno != ONEWIRE_NO_ERROR)
                    return FALSE;

                flag = FALSE;
                this->SelectElm = 0;

                for (int i = 0; i < ONEWIRESLAVE_COUNT; i++)
                {
                    if (this->elms[i] == nullptr) continue;

                    flag = TRUE;
                    for (int j = 0; j < 8; j++)
                    {
                        if (this->elms[i]->ID[j] != addr[j])
                        {
                            flag = FALSE;
                            break;
                        }
                    }

                    if (flag)
                    {
                        this->SelectElm = elms[i];

                        if (dbg_MATCHROM)
                        {
                            Serial.print("MATCH ROM=");
                            Serial.println(i);
                        }

                        break;
                    }
                }

                if (flag == FALSE) return FALSE;

                if (this->SelectElm != 0)
                    this->SelectElm->duty(this);

                return TRUE;
                break;

                // SKIP ROM
            case 0xCC:
                this->SelectElm = 0x00;
                return TRUE;

            default: // Unknow command
                Serial.print("U:");
                Serial.println(cmd, HEX);

                return FALSE;
                break;
        }
    }
}

uint8_t OneWireHub::sendData(uint8_t buf[], uint8_t len)
{
    uint8_t bytes_sended = 0;

    for (int i = 0; i < len; i++)
    {
        send(buf[i]);
        if (errno != ONEWIRE_NO_ERROR)
            break;
        bytes_sended++;
    }
    return bytes_sended;
}

uint8_t OneWireHub::recvData(uint8_t buf[], uint8_t len)
{
    uint8_t bytes_received = 0;

    for (int i = 0; i < len; i++)
    {
        buf[i] = recv();
        if (errno != ONEWIRE_NO_ERROR)
            break;
        bytes_received++;
    }
    return bytes_received;
}

void OneWireHub::send(uint8_t v)
{
    errno = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask && (errno == ONEWIRE_NO_ERROR); bitmask <<= 1)
        sendBit((bitmask & v) ? 1 : 0);
}

uint8_t OneWireHub::recv(void)
{
    uint8_t r = 0;

    errno = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask && (errno == ONEWIRE_NO_ERROR); bitmask <<= 1)
        if (recvBit())
            r |= bitmask;
    return r;
}

void OneWireHub::sendBit(uint8_t v)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;

    cli();
    DIRECT_MODE_INPUT(reg, mask);
    //waitTimeSlot waits for a low to high transition followed by a high to low within the time-out
    uint8_t wt = waitTimeSlot();
    if (wt != 1)
    { //1 is success, others are failure
        if (wt == 10)
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW;
        } else
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH;
        }
        sei();
        return;
    }
    if (v & 1)
        delayMicroseconds(30);
    else
    {
        cli();
        DIRECT_WRITE_LOW(reg, mask);
        DIRECT_MODE_OUTPUT(reg, mask);
        delayMicroseconds(30);
        DIRECT_WRITE_HIGH(reg, mask);
        sei();
    }
    sei();
    return;
}

uint8_t OneWireHub::recvBit(void)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint8_t r;

    cli();
    DIRECT_MODE_INPUT(reg, mask);
    //waitTimeSlotRead is a customized version of the original which was also
    // used by the "write" side of things.
    uint8_t wt = waitTimeSlotRead();
    if (wt != 1)
    { //1 is success, others are failure
        if (wt == 10)
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW;
        } else
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH;
        }
        sei();
        return 0;
    }
    delayMicroseconds(30);
    //TODO Consider reading earlier: delayMicroseconds(15);
    r = DIRECT_READ(reg, mask);
    sei();
    return r;
}

uint8_t OneWireHub::waitTimeSlot(void)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint16_t retries;

    //Wait for a 0 to rise to 1 on the line for timeout duration
    //If the line is already high, this is basically skipped
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    //While line is low, retry
    while (!DIRECT_READ(reg, mask))
        if (--retries == 0)
            return 10;

    //Wait for a fall form 1 to 0 on the line for timeout duration
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    while (DIRECT_READ(reg, mask));
    if (--retries == 0)
        return 20;

    return 1;
}

//This is a copy of what was orig just "waitTimeSlot"
// it is customized for the reading side of things
uint8_t OneWireHub::waitTimeSlotRead()
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint16_t retries;

    //Wait for a 0 to rise to 1 on the line for timeout duration
    //If the line is already high, this is basically skipped
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    //While line is low, retry
    while (!DIRECT_READ(reg, mask))
        if (--retries == 0)
            return 10;

    //TODO Seems to me that the above loop should drop out immediately because
    // The line is already high as our wait after presence is relatively short
    // So now it just waits a short period for the write of a bit to start
    // Unfortunately per "recommended" this is 55 micros to 130 micros more
    // more than what we may have already waited.

    //Wait for a fall form 1 to 0 on the line for timeout duration
    retries = TIMESLOT_WAIT_READ_RETRY_COUNT;
    while (DIRECT_READ(reg, mask));
    if (--retries == 0)
        return 20;

    return 1;
}

//==================== Item =========================================
OneWireItem::OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7)
{
    this->ID[0] = ID1;
    this->ID[1] = ID2;
    this->ID[2] = ID3;
    this->ID[3] = ID4;
    this->ID[4] = ID5;
    this->ID[5] = ID6;
    this->ID[6] = ID7;
    this->ID[7] = crc8(this->ID, 7);
}

#if ONEWIRESLAVE_CRC
// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
//

#if ONEWIRESLAVE_CRC8_TABLE
// This table comes from Dallas sample code where it is freely reusable,
// though Copyright (C) 2000 Dallas Semiconductor Corporation
static const uint8_t PROGMEM dscrc_table[] = {
      0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
    157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
     35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
    190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
     70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
    219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
    101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
    248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
    140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
     17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
    175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
     50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
    202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
     87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
    233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
    116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

//
// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (note: this might better be done without to
// table, it would probably be smaller and certainly fast enough
// compared to all those delayMicrosecond() calls.  But I got
// confused, so I use this table from the examples.)
//
uint8_t OneWireItem::crc8(char addr[], uint8_t len)
{
    uint8_t crc = 0;

    while (len--) {
        crc = pgm_read_byte(dscrc_table + (crc ^ *addr++));
    }
    return crc;
}
#else

//
// Compute a Dallas Semiconductor 8 bit CRC directly.
//
uint8_t OneWireItem::crc8(uint8_t addr[], uint8_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        uint8_t inbyte = *addr++;
        for (uint8_t i = 8; i; i--)
        {
            uint8_t mix = (crc ^ inbyte) & 0x01;
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

uint16_t OneWireItem::crc16(uint8_t addr[], uint8_t len)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < len; i++)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = addr[i];
        cdata = (cdata ^ crc) & 0xff;
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
}


#endif
#endif

