
#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#endif
#include "platform.h" // code for compatibility

#define USE_SERIAL_DEBUG 0 // give debug messages when printError() is called
// INFO: had to go with a define because some compilers use constexpr as simple const --> massive problems

#define HUB_SLAVE_LIMIT 8 // set the limit of the hub HERE


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

using time_t = uint32_t;

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
};


class OneWireItem;

class OneWireHub
{
private:

    static constexpr uint8_t ONEWIRESLAVE_LIMIT                 = HUB_SLAVE_LIMIT;
    static constexpr uint8_t ONEWIRE_TREE_SIZE                  = 2*ONEWIRESLAVE_LIMIT - 1;


    /// the following TIME-values are in microseconds and are taken from the ds2408 datasheet
    // should be --> datasheet
    // was       --> shagrat-legacy
    static constexpr uint16_t ONEWIRE_TIME_BUS_CHANGE_MAX       =    5; //

    static constexpr uint16_t ONEWIRE_TIME_RESET_MIN            =  430; // should be 480, and was 470
    static constexpr uint16_t ONEWIRE_TIME_RESET_MAX            =  960; // from ds2413

    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_SAMPLE_MIN  =   20; // probe measures 40us
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_STD     =  160; // was 125
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_LOW_MAX     =  480; // should be 280, was 480 !!!! why
    static constexpr uint16_t ONEWIRE_TIME_PRESENCE_HIGH_MAX    =20000; //

    static constexpr uint16_t ONEWIRE_TIME_SLOT_MAX             =  135; // should be 120, was ~1050

    // read and write from the viewpoint of the slave!!!!
    static constexpr uint16_t ONEWIRE_TIME_READ_ONE_LOW_MAX     =   60; //
    static constexpr uint16_t ONEWIRE_TIME_READ_STD             =   20; // was 30
    static constexpr uint16_t ONEWIRE_TIME_WRITE_ZERO_LOW_STD   =   35; //
    // TODO: use #define to switch to overdrive mode

    time_t LOOPS_BUS_CHANGE_MAX;
    time_t LOOPS_RESET_MIN;
    time_t LOOPS_RESET_MAX;
    time_t LOOPS_PRESENCE_SAMPLE_MIN;
    time_t LOOPS_PRESENCE_LOW_STD;
    time_t LOOPS_PRESENCE_LOW_MAX;
    time_t LOOPS_PRESENCE_HIGH_MAX;
    time_t LOOPS_SLOT_MAX;
    time_t LOOPS_READ_ONE_LOW_MAX;
    time_t LOOPS_READ_STD;
    time_t LOOPS_WRITE_ZERO_LOW_STD;


    Error   _error;
    uint8_t _error_cmd;

    uint8_t           pin_bitMask;
    volatile uint8_t *pin_baseReg;
    uint8_t           extend_timeslot_detection;
    uint8_t           skip_reset_detection;


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

    uint8_t getNrOfFirstBitSet(const mask_t mask);
    uint8_t getNrOfFirstFreeIDTreeElement(void);

    bool checkReset(uint16_t timeout_us);

    bool showPresence(void);

    bool search(void);

    bool recvAndProcessCmd();

    __attribute__((always_inline))
    void wait(const uint16_t timeout_us);

    __attribute__((always_inline))
    bool awaitTimeSlotAndWrite(const bool writeZero = 0);

    __attribute__((always_inline))
    bool waitWhilePinIs(const bool value, const uint16_t timeout_us);

public:

    explicit OneWireHub(uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    bool poll(void);

    [[deprecated("use the non-blocking poll() instead of waitForRequest()")]]
    bool waitForRequest(const bool ignore_errors = false);

    void extendTimeslot(void);

    bool send(const uint8_t dataByte);
    bool send(const uint8_t address[], const uint8_t data_length);
    bool sendBit(const bool value);

    // CRC takes ~7.4µs/byte (Atmega328P@16MHz) but is distributing the load between each bit-send to 0.9 µs/bit (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!
    uint16_t sendAndCRC16(uint8_t dataByte, uint16_t crc16);

    uint8_t recv(void);
    bool    recv(uint8_t address[], const uint8_t data_length); // TODO: change send/recv to return bool TRUE on success, recv returns data per reference
    bool    recvBit(void);
    uint8_t recvAndCRC16(uint16_t &crc16);

    void printError(void);
    bool getError(void);
    void raiseSlaveError(const uint8_t cmd = 0);

};



#endif