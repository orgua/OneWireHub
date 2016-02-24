
#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#include "Arduino.h"

constexpr bool dbg_IDTREE   = 0; // give debug messages
constexpr bool dbg_SEARCH   = 0; // give debug messages
constexpr bool dbg_MATCHROM = 0; // give debug messages
constexpr bool dbg_HINT     = 0; // give debug messages for called unimplemented functions of sensors

// TODO: rework this whole system
// - cleaner timing-system throughout the lib (raw tick-counter of micros()?)
// - offer strict and relaxed timing option (if master is also emulated)
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
    static constexpr uint8_t ONEWIRE_PRESENCE_HIGH_ON_LINE      = 9;

    /// the following TIME-values are in us and are taken from the ds2408 datasheet
    static constexpr uint16_t ONEWIRE_TIME_BUS_CHANGE_MAX       =   5; // used

    static constexpr uint16_t ONEWIRE_TIME_RESET_MIN            = 380; // used, was 480
    static constexpr uint16_t ONEWIRE_TIME_RESET_MAX            = 720; // used

    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MIN    =  15;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MAX    =  60;
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_SAMPLE_MIN  =  30; // used
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

    uint8_t           pin_bitMask;
    volatile uint8_t *baseReg;

    uint8_t      slave_count;
    OneWireItem *slave_list[ONEWIRESLAVE_LIMIT];  // private slave-list (use attach/detach)
    OneWireItem *slave_selected;

    struct IDTree {
        uint8_t slave_selected; // for which slave is this jump-command relevant
        uint8_t id_position;    // where does the algorithm has to look for a junction
        uint8_t got_zero;        // if 0 switch to which tree branch
        uint8_t got_one;         // if 1 switch to which tree branch
    } idTree[ONEWIRETREE_SIZE];

    uint8_t buildIDTree(void);
    uint8_t buildIDTree(uint8_t position_IDBit, const uint8_t slave_mask);

    uint8_t  getNrOfFirstBitSet(const uint8_t mask);
    uint16_t getNrOfFirstBitSet(const uint16_t mask);
    uint8_t  getNrOfFirstFreeIDTreeElement(void);

    bool recvAndProcessCmd();

    bool waitTimeSlot();

    bool checkReset(uint16_t timeout_us);

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

    bool send(const uint8_t dataByte);
    bool send(const uint8_t address[], const uint8_t data_length);
    bool sendBit(const bool value);
    uint16_t sendAndCRC16(uint8_t dataByte, uint16_t crc16);

    uint8_t recv(void);
    bool    recv(uint8_t address[], const uint8_t data_length); // TODO: change send/recv to return bool TRUE on success, recv returns data per reference
    uint8_t recvBit(void);

    void printError(void);

    bool error(void)
    {
        if (_error == ONEWIRE_NO_ERROR) return 0;
        return 1;
    };

};



#endif