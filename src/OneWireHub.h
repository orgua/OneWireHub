#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#endif

#include "platform.h" // code for compatibility
#include "OneWireHub_config.h" // outsource configfile


#ifndef HUB_SLAVE_LIMIT
#error "Slavelimit not defined (why?)"
#elif (HUB_SLAVE_LIMIT > 32)
#error "Slavelimit is set to high (32)"
#elif (HUB_SLAVE_LIMIT > 16)
using mask_t = uint32_t;
#elif (HUB_SLAVE_LIMIT > 8)
using mask_t = uint16_t;
#elif (HUB_SLAVE_LIMIT > 0)
using mask_t = uint8_t;
#else
#error "Slavelimit is set to zero (why?)"
#endif

using     timeOW_t = uint32_t;

constexpr timeOW_t VALUE1k      {1000}; // commonly used constant
constexpr timeOW_t TIMEOW_MAX   {4294967295};   // arduino does not support std-lib...

enum class Error : uint8_t {
    NO_ERROR                   = 0,
    READ_TIMESLOT_TIMEOUT      = 1,
    WRITE_TIMESLOT_TIMEOUT     = 2,
    WAIT_RESET_TIMEOUT         = 3,
    VERY_LONG_RESET            = 4,
    VERY_SHORT_RESET           = 5,
    PRESENCE_LOW_ON_LINE       = 6,
    READ_TIMESLOT_TIMEOUT_LOW  = 7,
    READ_TIMESLOT_TIMEOUT_HIGH = 8,
    PRESENCE_HIGH_ON_LINE      = 9,
    INCORRECT_ONEWIRE_CMD      = 10,
    INCORRECT_SLAVE_USAGE      = 11,
    TRIED_INCORRECT_WRITE      = 12,
    FIRST_TIMESLOT_TIMEOUT     = 13,
    FIRST_BIT_OF_BYTE_TIMEOUT  = 14,
    RESET_IN_PROGRESS          = 15
};


constexpr timeOW_t timeUsToLoops(const uint16_t time_us)
{
    return (time_us * microsecondsToClockCycles(1) / VALUE_IPL); // note: microsecondsToClockCycles is speed in MHz....
};

#if OVERDRIVE_ENABLE
static constexpr timeOW_t LOOPS_RESET_TIMEOUT          = timeUsToLoops(ONEWIRE_TIME_RESET_TIMEOUT);
static constexpr timeOW_t LOOPS_RESET_MIN[2]           = { timeUsToLoops(ONEWIRE_TIME_RESET_MIN),        timeUsToLoops(OVERDRIVE_TIME_RESET_MIN) };
static constexpr timeOW_t LOOPS_RESET_MAX[2]           = { timeUsToLoops(ONEWIRE_TIME_RESET_MAX),        timeUsToLoops(OVERDRIVE_TIME_RESET_MAX) };
static constexpr timeOW_t LOOPS_PRESENCE_TIMEOUT[2]    = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_TIMEOUT), timeUsToLoops(OVERDRIVE_TIME_PRESENCE_TIMEOUT) };
static constexpr timeOW_t LOOPS_PRESENCE_MIN[2]        = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_MIN),     timeUsToLoops(OVERDRIVE_TIME_PRESENCE_MIN) };
static constexpr timeOW_t LOOPS_PRESENCE_MAX[2]        = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_MAX),     timeUsToLoops(OVERDRIVE_TIME_PRESENCE_MAX) };
static constexpr timeOW_t LOOPS_MSG_HIGH_TIMEOUT       = timeUsToLoops(ONEWIRE_TIME_MSG_HIGH_TIMEOUT);
static constexpr timeOW_t LOOPS_SLOT_MAX[2]            = { timeUsToLoops(ONEWIRE_TIME_SLOT_MAX),         timeUsToLoops(OVERDRIVE_TIME_SLOT_MAX) };
static constexpr timeOW_t LOOPS_READ_MIN[2]            = { timeUsToLoops(ONEWIRE_TIME_READ_MIN),         timeUsToLoops(OVERDRIVE_TIME_READ_MIN) };
static constexpr timeOW_t LOOPS_READ_MAX[2]            = { timeUsToLoops(ONEWIRE_TIME_READ_MAX),         timeUsToLoops(OVERDRIVE_TIME_READ_MAX) };
static constexpr timeOW_t LOOPS_WRITE_ZERO[2]          = { timeUsToLoops(ONEWIRE_TIME_WRITE_ZERO),       timeUsToLoops(OVERDRIVE_TIME_WRITE_ZERO) };
#else
static constexpr timeOW_t LOOPS_RESET_TIMEOUT          = timeUsToLoops(ONEWIRE_TIME_RESET_TIMEOUT);
static constexpr timeOW_t LOOPS_RESET_MIN[1]           = { timeUsToLoops(ONEWIRE_TIME_RESET_MIN) };
static constexpr timeOW_t LOOPS_RESET_MAX[1]           = { timeUsToLoops(ONEWIRE_TIME_RESET_MAX) };
static constexpr timeOW_t LOOPS_PRESENCE_TIMEOUT[1]    = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_TIMEOUT) };
static constexpr timeOW_t LOOPS_PRESENCE_MIN[1]        = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_MIN) };
static constexpr timeOW_t LOOPS_PRESENCE_MAX[1]        = { timeUsToLoops(ONEWIRE_TIME_PRESENCE_MAX) };
static constexpr timeOW_t LOOPS_MSG_HIGH_TIMEOUT       = timeUsToLoops(ONEWIRE_TIME_MSG_HIGH_TIMEOUT);
static constexpr timeOW_t LOOPS_SLOT_MAX[1]            = { timeUsToLoops(ONEWIRE_TIME_SLOT_MAX) };
static constexpr timeOW_t LOOPS_READ_MIN[1]            = { timeUsToLoops(ONEWIRE_TIME_READ_MIN) };
static constexpr timeOW_t LOOPS_READ_MAX[1]            = { timeUsToLoops(ONEWIRE_TIME_READ_MAX) };
static constexpr timeOW_t LOOPS_WRITE_ZERO[1]          = { timeUsToLoops(ONEWIRE_TIME_WRITE_ZERO) };
#endif

class OneWireItem;

class OneWireHub
{
private:

    static constexpr uint8_t ONEWIRESLAVE_LIMIT                 = HUB_SLAVE_LIMIT;
    static constexpr uint8_t ONEWIRE_TREE_SIZE                  = ( 2 * ONEWIRESLAVE_LIMIT ) - 1;

#if OVERDRIVE_ENABLE
    bool od_mode;
#else
    static constexpr bool od_mode = false;
#endif

    Error   _error;
    uint8_t _error_cmd;

    io_reg_t          pin_bitMask;
    volatile io_reg_t *pin_baseReg;

#if 1 //USE_GPIO_DEBUG
    io_reg_t          debug_bitMask;
    volatile io_reg_t *debug_baseReg;
#endif

    uint8_t      slave_count;
    OneWireItem *slave_list[ONEWIRESLAVE_LIMIT];  // private slave-list (use attach/detach)
    OneWireItem *slave_selected;

    struct IDTree {
        uint8_t slave_selected; // for which slave is this jump-command relevant
        uint8_t id_position;    // where does the algorithm has to look for a junction
        uint8_t got_zero;        // if 0 switch to which tree branch
        uint8_t got_one;         // if 1 switch to which tree branch
    } idTree[ONEWIRE_TREE_SIZE];

    uint8_t buildIDTree(void);
    uint8_t buildIDTree(uint8_t position_IDBit, const mask_t slave_mask);
    void    searchIDTree(void);

    uint8_t getNrOfFirstBitSet(const mask_t mask) const;
    uint8_t getNrOfFirstFreeIDTreeElement(void) const;

    bool checkReset(void);      // returns 1 if error occured
    bool showPresence(void);    // returns 1 if error occured
    bool recvAndProcessCmd();   // returns 1 if error occured

    void wait(const timeOW_t loops_wait) const;
    void wait(const uint16_t timeout_us) const;

    inline __attribute__((always_inline))
    timeOW_t waitLoopsWhilePinIs(volatile timeOW_t retries, const bool pin_value = false) const;

public:

    explicit OneWireHub(const uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    uint8_t getIndexOfNextSensorInList(const uint8_t index_start = 0) const;

    bool poll(void);

    bool sendBit(const bool value);                                 // returns 1 if error occured
    bool send(const uint8_t dataByte);                              // returns 1 if error occured
    bool send(const uint8_t address[], const uint8_t data_length);  // returns 1 if error occured
    bool send(const uint8_t address[], const uint8_t data_length, uint16_t &crc16);  // returns 1 if error occured
    // CRC takes ~7.4µs/byte (Atmega328P@16MHz) but is distributing the load between each bit-send to 0.9 µs/bit (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    [[deprecated("Replaced by send(const uint8_t address[], const uint8_t data_length, uint16_t &crc16)")]]
    uint16_t sendAndCRC16(uint8_t dataByte, uint16_t crc16);

    bool    recvBit(void);
    bool    recv(uint8_t address[], const uint8_t data_length); // returns 1 if error occured
    bool    recv(uint8_t address[], const uint8_t data_length, uint16_t &crc16); // returns 1 if error occured
    [[deprecated("Replaced by recv(uint8_t address[], const uint8_t data_length)")]]
    uint8_t recv(void);
    [[deprecated("Replaced by recv(uint8_t address[], const uint8_t data_length, uint16_t &crc16))")]]
    uint8_t recvAndCRC16(uint16_t &crc16);

    timeOW_t waitLoopsCalibrate(void); // returns Instructions per loop
    void     waitLoops1ms(void);
    void     waitLoopsDebug(void) const;

    // mostly for debug, partly for state-machine handling
    void printError(void) const;
    bool getError(void) const; // returns 1 if error occured
    void raiseSlaveError(const uint8_t cmd = 0);
    Error clearError(void);

};



#endif