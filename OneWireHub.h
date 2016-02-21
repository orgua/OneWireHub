
#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#include "Arduino.h"
#include <inttypes.h>

constexpr bool dbg_CALC     = 0; // give debug messages
constexpr bool dbg_SEARCH   = 0; // give debug messages
constexpr bool dbg_MATCHROM = 0; // give debug messages
constexpr bool dbg_HINT     = 0; // give debug messages for called unimplemented functions of sensors

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
    uint8_t errno; // TODO: rename to error_raised
    uint8_t pin_bitmask; // TODO: is it used? every function seems to define its local version
    uint8_t slave_count;

    volatile uint8_t *baseReg; // TODO: is it used? every function seems to define its local version

    struct IDTree {
        uint8_t slave_selected; // for which slave is this jump-command relevant
        uint8_t bitposition;    // where does the algo has to look for a junction
        uint8_t gotZero;        // if 0 switch to which tree branch
        uint8_t gotOne;         // if 1 switch to which tree branch
    } idTree[ONEWIRETREE_SIZE];

    OneWireItem *elms[ONEWIRESLAVE_LIMIT];  // private slave-list (use attach/detach)

    OneWireItem *SelectElm; // TODO: rename to slave_list[] and slave_selected

    bool recvAndProcessCmd();

    uint8_t waitTimeSlot();

    int calc_mask(void); // TODO: rename to buildIDTree() and same with arguments below
    uint8_t build_tree(uint8_t bitposition, const uint8_t slave_mask);
    uint8_t get_first_element(const uint8_t mask); // TODO: rename to getNrOfFirstBitSet

    bool waitReset(uint16_t timeout_ms = 1000); // TODO: maybe tune here to make all sensors appear in search

    bool presence(const uint8_t delta_us = 25);

    bool search(void);

    uint8_t recvBit(void);

public:

    explicit OneWireHub(uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    bool waitForRequest(const bool ignore_errors = false);

    void send(const uint8_t v);

    uint8_t sendData(const uint8_t buf[], const uint8_t data_len); // TODO: rename to send()

    void sendBit(const uint8_t v);

    uint8_t recv(void);

    uint8_t recvData(uint8_t buf[], const uint8_t data_len); // TODO: recv()

    bool error(void)
    {
        if (errno == ONEWIRE_NO_ERROR) return 0;
        return 1;
    };

};

// TODO: get first byte (family code) const for every sensor
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
};

void ow_crc16_reset(void);

void ow_crc16_update(uint8_t b);

uint16_t ow_crc16_get(void);

#endif