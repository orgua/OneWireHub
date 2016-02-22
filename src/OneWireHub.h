
#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#include "Arduino.h"
#include <inttypes.h>

constexpr bool dbg_CALC     = 0; // give debug messages
constexpr bool dbg_SEARCH   = 0; // give debug messages
constexpr bool dbg_MATCHROM = 0; // give debug messages
constexpr bool dbg_HINT     = 0; // give debug messages for called unimplemented functions of sensors

// TODO: rework this whole system
// - cleaner timing-system throughout the lib (raw tick-counter of micros()?)
// - rename and adapt all functions
// - offer strict and relaxed timing option (if master is also emulated)
// - remodel the bus-specific parts
// - offer interruptable read / write OPs
// - safe timestamp of last HIGH, LOW state?

class OneWireItem;

class OneWireHub
{
private:

    static constexpr uint8_t ONEWIRESLAVE_LIMIT                 = 8; // 8 is max at the moment, need bigger vars on some loops
    static constexpr uint8_t ONEWIRETREE_SIZE                   = 2*ONEWIRESLAVE_LIMIT - 1;

    static constexpr uint8_t ONEWIRE_NO_ERROR                   = 0;
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT      = 1;
    static constexpr uint8_t ONEWIRE_WRITE_TIMESLOT_TIMEOUT     = 2;
    static constexpr uint8_t ONEWIRE_WAIT_RESET_TIMEOUT         = 3;
    static constexpr uint8_t ONEWIRE_VERY_LONG_RESET            = 4;
    static constexpr uint8_t ONEWIRE_VERY_SHORT_RESET           = 5;
    static constexpr uint8_t ONEWIRE_PRESENCE_LOW_ON_LINE       = 6;
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW  = 7;
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH = 8;

    /// the following TIME-values are in us and are taken from the ds2408 datasheet
    static constexpr uint16_t ONEWIRE_TIME_RESET_MIN            = 480;
    static constexpr uint16_t ONEWIRE_TIME_RESET_MAX            = 720;

    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MIN    =  15;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MAX    =  60;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MIN     =  60;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MAX     = 280;

    static constexpr uint16_t ONEWIRE_TIME_SLOT_MIN             =  65;
    static constexpr uint16_t ONEWIRE_TIME_SLOT_MAX             = 120;

    static constexpr uint16_t ONEWIRE_TIME_WRITE_ZERO_LOW_MIN   =  60;
    static constexpr uint16_t ONEWIRE_TIME_WRITE_ZERO_LOW_MAX   = 120;
    static constexpr uint16_t ONEWIRE_TIME_WRITE_ONE_LOW_MIN    =  15;
    static constexpr uint16_t ONEWIRE_TIME_WRITE_ONE_LOW_MAX    =  60;

    static constexpr uint16_t ONEWIRE_TIME_READ_LOW_MIN         =   5;
    static constexpr uint16_t ONEWIRE_TIME_READ_LOW_MAX         =  15;
    static constexpr uint16_t ONEWIRE_TIME_READ_ZERO_LOW_MIN    =  15;
    static constexpr uint16_t ONEWIRE_TIME_READ_ZERO_LOW_MAX    =  60;

    uint8_t _error;

    uint8_t           pin_bitmask; // TODO: is it used? every function seems to define its local version
    volatile uint8_t *baseReg; // TODO: is it used? every function seems to define its local version

    uint8_t      slave_count;
    OneWireItem *slave_list[ONEWIRESLAVE_LIMIT];  // private slave-list (use attach/detach)
    OneWireItem *slave_selected;

    struct IDTree {
        uint8_t slave_selected; // for which slave is this jump-command relevant
        uint8_t idPosition;    // where does the algo has to look for a junction
        uint8_t gotZero;        // if 0 switch to which tree branch
        uint8_t gotOne;         // if 1 switch to which tree branch
    } idTree[ONEWIRETREE_SIZE];

    uint8_t buildIDTree(void);
    uint8_t buildIDTree(uint8_t position_IDBit, const uint8_t slave_mask);

    uint8_t  getNrOfFirstBitSet(const uint8_t mask);
    uint16_t getNrOfFirstBitSet(const uint16_t mask);

    bool recvAndProcessCmd();

    uint8_t waitTimeSlot();

    bool waitReset(uint16_t timeout_ms = 1000); // TODO: maybe tune here to make all sensors appear in search

    bool presence(const uint8_t delta_us = 25);

    bool search(void);



public:

    explicit OneWireHub(uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    bool poll(void);

    [[deprecated("use the non-blocking poll() instead of waitForRequest()")]]
    bool waitForRequest(const bool ignore_errors = false);

    uint8_t send(const uint8_t databyte);
    uint8_t send(const uint8_t buf[], const uint8_t data_len);
    uint8_t sendBit(const uint8_t v);

    uint8_t recv(void);
    uint8_t recv(uint8_t buf[], const uint8_t data_len);
    uint8_t recvBit(void);

    bool error(void)
    {
        if (_error == ONEWIRE_NO_ERROR) return 0;
        return 1;
    };



};

/*
//--- CRC 16 --- // TODO: only used in ds2450 and ds2408 and ds2423, but integrate into hub as static --> turn to bitwise sendAndCRC()
static uint16_t crc16;

void ow_crc16_reset(void)
{
    crc16 = 0;
}

void ow_crc16_update(uint8_t b)
{
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint8_t mix = ((uint8_t) crc16 ^ b) & static_cast<uint8_t>(0x01);
        crc16 = crc16 >> 1;
        if (mix)
            crc16 = crc16 ^ static_cast<uint16_t>(0xA001);

        b = b >> 1;
    }
}

uint16_t ow_crc16_get(void)
{
    return crc16;
}*/

// Feature: get first byte (family code) constant for every sensor --> var4 is implemented
// - var 1: use second init with one byte less (Serial 1-6 instead of ID)
// - var 2: write ID1 of OneWireItem with the proper value without asking
// - var 3: rewrite the OneWireItem-Class and implement something like setFamilyCode()
// - var 4: make public family_code in sensor mandatory and just put it into init() if wanted --> prefer this

class OneWireItem
{
public:

    OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t ID[8];

    virtual bool duty(OneWireHub *hub) = 0; // TODO: duty in sensors is set private... but it does work anyway

    static uint8_t crc8(const uint8_t addr[], const uint8_t len);

    static uint16_t crc16(const uint8_t addr[], const uint8_t len);

    // CRC16 of type 0xC001 for little endian
    static uint16_t crc16(uint16_t crc, uint8_t value) // TODO: further tuning with asm
    {
        static const uint8_t oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
        value = (value ^ static_cast<uint8_t>(crc));
        crc >>= 8;
        if (oddparity[value & 0x0F] ^ oddparity[value >> 4])   crc ^= 0xC001;
        uint16_t cdata = (static_cast<uint16_t>(value) << 6);
        crc ^= cdata;
        crc ^= (static_cast<uint16_t>(cdata) << 1);
        return crc;
    };

};

#endif