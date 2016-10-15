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
    FIRST_BIT_OF_BYTE_TIMEOUT  = 14
};


class OneWireItem;

class OneWireHub
{
private:

    static constexpr uint8_t ONEWIRESLAVE_LIMIT                 = HUB_SLAVE_LIMIT;
    static constexpr uint8_t ONEWIRE_TREE_SIZE                  = 2*ONEWIRESLAVE_LIMIT - 1;

    timeOW_t value_nspl; // nanoseconds per loop
    timeOW_t loops_bus_change_max;
    timeOW_t loops_reset_min;
    timeOW_t loops_reset_max;
    timeOW_t loops_reset_timeout;
    timeOW_t loops_presence_sample_min;
    timeOW_t loops_presence_low_std;
    timeOW_t loops_presence_low_max;
    timeOW_t loops_presence_high_max;
    timeOW_t loops_slot_max;
    timeOW_t loops_read_one_low_max;
    timeOW_t loops_read_std;
    timeOW_t loops_write_zero_low_std;

    Error   _error;
    uint8_t _error_cmd;

    io_reg_t          pin_bitMask;
    volatile io_reg_t *pin_baseReg;

#if USE_GPIO_DEBUG
    io_reg_t          debug_bitMask;
    volatile io_reg_t *debug_baseReg;
#endif

    uint8_t           extend_timeslot_detection;
    uint8_t           skip_reset_detection;

    bool              calibrate_loop_timing;
    bool              overdrive_mode;

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

    bool checkReset(void);

    bool showPresence(void);

    bool search(void);

    bool recvAndProcessCmd();

    inline __attribute__((always_inline))
    void wait(const uint16_t timeout_us);

    inline __attribute__((always_inline))
    bool awaitTimeSlotAndWrite(const bool writeZero = 0);

    timeOW_t waitLoopsCalculate(const timeOW_t time_ns);
    void waitLoopsConfig(void);

public:

    explicit OneWireHub(const uint8_t pin, const uint8_t value_ipl = 1); // std: set instructions per loop to 1 to read from platform.h

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    uint8_t getIndexOfNextSensorInList(const uint8_t index_start = 0);

    bool poll(void);

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

    timeOW_t waitLoopsCalibrate(void); // returns Instructions per loop
    inline __attribute__((always_inline))
    timeOW_t waitLoopsWhilePinIs(volatile timeOW_t retries, const bool pin_value = false);
    void waitLoopsDebug(void);

    void printError(void);
    bool getError(void);
    void raiseSlaveError(const uint8_t cmd = 0);
    Error clearError(void);

};



#endif