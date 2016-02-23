
#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#include "Arduino.h"
#include <inttypes.h>

constexpr bool dbg_IDTREE   = 0; // give debug messages
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

    static constexpr uint8_t ONEWIRE_NO_ERROR                   = 0; // TODO: could be a enum
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT      = 1;
    static constexpr uint8_t ONEWIRE_WRITE_TIMESLOT_TIMEOUT     = 2;
    static constexpr uint8_t ONEWIRE_WAIT_RESET_TIMEOUT         = 3;
    static constexpr uint8_t ONEWIRE_VERY_LONG_RESET            = 4;
    static constexpr uint8_t ONEWIRE_VERY_SHORT_RESET           = 5;
    static constexpr uint8_t ONEWIRE_PRESENCE_LOW_ON_LINE       = 6;
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW  = 7;
    static constexpr uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH = 8;

    /// the following TIME-values are in us and are taken from the ds2408 datasheet
    static constexpr uint16_t ONEWIRE_TIME_BUS_CHANGE_MAX       =   5; // used

    static constexpr uint16_t ONEWIRE_TIME_RESET_MIN            = 480; // used
    static constexpr uint16_t ONEWIRE_TIME_RESET_MAX            = 720; // used

    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MIN    =  15;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_STD    =  30; // used
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MAX    =  60;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MIN     =  60;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_STD     = 140; // used
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MAX     = 280; // used

    static constexpr uint16_t ONEWIRE_TIME_SLOT_MIN             =  65;
    static constexpr uint16_t ONEWIRE_TIME_SLOT_MAX             = 120; // was 120

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

    bool waitTimeSlot();

    bool checkReset(uint16_t timeout_us = 2000);

    bool showPresence(void);

    bool search(void);

public:

    explicit OneWireHub(uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    bool poll(void);

    [[deprecated("use the non-blocking poll() instead of waitForRequest()")]]
    bool waitForRequest(const bool ignore_errors = false);

    bool send(const uint8_t databyte);
    bool send(const uint8_t buf[], const uint8_t data_len);
    bool sendBit(const uint8_t v);
    uint16_t sendAndCRC16(uint8_t databyte, uint16_t crc16);

    uint8_t recv(void);
    uint8_t recv(uint8_t buf[], const uint8_t data_len); // TODO: change send/recv to return bool TRUE on success, recv returns data per reference
    uint8_t recvBit(void);

    void printError(void);

    bool error(void)
    {
        if (_error == ONEWIRE_NO_ERROR) return 0;
        return 1;
    };



};

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

    // takes ~(5.1-7.0)µs/byte (Atmega328P@16MHz) depends from addr_size (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(const uint8_t addr[], const uint8_t len);

    // CRC16 of type 0xC001 for little endian
    // takes ~6µs/byte (Atmega328P@16MHz) (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    static uint16_t crc16(uint8_t value, uint16_t crc) // TODO: further tuning with asm
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