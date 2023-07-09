#ifndef ONEWIRE_HUB_H
#define ONEWIRE_HUB_H

#include "platform.h" // code for compatibility

using timeOW_t = uint32_t;
constexpr timeOW_t timeOW_max =
        4294967295; // will arduino-gcc ever offer some stl? std::numeric_limits::max would be cleaner

constexpr timeOW_t
operator"" _us(const unsigned long long int time_us) // user defined literal used in config
{
    return timeOW_t(time_us * microsecondsToClockCycles(1) /
                    VALUE_IPL); // note: microsecondsToClockCycles == speed in MHz....
    // TODO: overflow detection would be nice, but literals are allowed with return-only, not solvable ATM
}


// same FN, but not as literal
constexpr timeOW_t timeUsToLoops(const uint16_t time_us)
{
    return (time_us * microsecondsToClockCycles(1) /
            VALUE_IPL); // note: microsecondsToClockCycles == speed in MHz....
}

#include "OneWireHub_config.h" // outsource configfile

#ifndef ONEWIREHUB_DEVICE_LIMIT
  #error "ONEWIREHUB_DEVICE_LIMIT not defined (why?)"
#elif (ONEWIREHUB_DEVICE_LIMIT > 32)
  #error "ONEWIREHUB_DEVICE_LIMIT is set too high (>32)"
#elif (ONEWIREHUB_DEVICE_LIMIT > 16)
using mask_t = uint32_t;
#elif (ONEWIREHUB_DEVICE_LIMIT > 8)
using mask_t = uint16_t;
#elif (ONEWIREHUB_DEVICE_LIMIT > 0)
using mask_t = uint8_t;
#else
  #error "DeviceLimit is set to zero (why?)"
#endif

#ifndef ONEWIREHUB_OVERDRIVE_ENABLE
  #error "ONEWIREHUB_OVERDRIVE_ENABLE not defined (why?)"
#elif !((ONEWIREHUB_OVERDRIVE_ENABLE >= 0) && (ONEWIREHUB_OVERDRIVE_ENABLE <= 1))
  #error "ONEWIREHUB_OVERDRIVE_ENABLE must be 0 or 1"
#endif

constexpr timeOW_t VALUE1k{1000};          // commonly used constant
constexpr timeOW_t TIMEOW_MAX{4294967295}; // arduino does not support std-lib...

enum class Error : uint8_t
{
    NO_ERROR                    = 0,
    READ_TIMESLOT_TIMEOUT       = 1,
    WRITE_TIMESLOT_TIMEOUT      = 2,
    WAIT_RESET_TIMEOUT          = 3,
    VERY_LONG_RESET             = 4,
    VERY_SHORT_RESET            = 5,
    PRESENCE_LOW_ON_LINE        = 6,
    READ_TIMESLOT_TIMEOUT_LOW   = 7,
    AWAIT_TIMESLOT_TIMEOUT_HIGH = 8,
    PRESENCE_HIGH_ON_LINE       = 9,
    INCORRECT_ONEWIRE_CMD       = 10,
    INCORRECT_DEVICE_USAGE      = 11,
    TRIED_INCORRECT_WRITE       = 12,
    FIRST_TIMESLOT_TIMEOUT      = 13,
    FIRST_BIT_OF_BYTE_TIMEOUT   = 14,
    RESET_IN_PROGRESS           = 15
};


class OneWireItem;

class OneWireHub
{
private:
    static constexpr uint8_t _ONEWIREHUB_DEVICE_LIMIT{ONEWIREHUB_DEVICE_LIMIT};
    static constexpr uint8_t _ONEWIREHUB_TREE_SIZE{(2 * _ONEWIREHUB_DEVICE_LIMIT) - 1};

#if ONEWIREHUB_OVERDRIVE_ENABLE
    bool od_mode;
#else
    static constexpr bool od_mode{false};
#endif

    Error   _error;
    uint8_t _error_cmd;

    io_reg_t           pin_bitMask;
    volatile io_reg_t *pin_baseReg;

#if true //USE_GPIO_DEBUG
    io_reg_t           debug_bitMask;
    volatile io_reg_t *debug_baseReg;
#endif

    uint8_t      device_count;
    OneWireItem *device_list[_ONEWIREHUB_DEVICE_LIMIT]; // private device-list (use attach/detach)
    OneWireItem *device_selected;

    struct IDTree
    {
        uint8_t device_selected; // for which device is this jump-command relevant
        uint8_t id_position;     // where does the algorithm has to look for a junction
        uint8_t got_zero;        // if 0 switch to which tree branch
        uint8_t got_one;         // if 1 switch to which tree branch
    } idTree[_ONEWIREHUB_TREE_SIZE];

    uint8_t buildIDTree(void);
    uint8_t buildIDTree(uint8_t position_IDBit, mask_t device_mask);
    void    searchIDTree(void);

    uint8_t getNrOfFirstBitSet(mask_t mask) const;
    uint8_t getNrOfFirstFreeIDTreeElement(void) const;

    bool checkReset(void);    // returns true if error occurred
    bool showPresence(void);  // returns true if error occurred
    bool recvAndProcessCmd(); // returns true if error occurred

    void wait(timeOW_t loops_wait) const;
    void wait(uint16_t timeout_us) const;

    inline __attribute__((always_inline)) timeOW_t
    waitLoopsWhilePinIs(volatile timeOW_t retries, bool pin_value = false) const;

public:
    explicit OneWireHub(uint8_t pin);

    ~OneWireHub() = default; // nothing special to do here

    OneWireHub(const OneWireHub &hub)            = delete;  // disallow copy constructor
    OneWireHub(OneWireHub &&hub)                 = default; // default move constructor
    OneWireHub &operator=(OneWireHub &hub)       = delete;  // disallow copy assignment
    OneWireHub &operator=(const OneWireHub &hub) = delete;  // disallow copy assignment
    OneWireHub &operator=(OneWireHub &&hub)      = delete;  // disallow move assignment

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(uint8_t device_number);

    uint8_t getIndexOfNextSensorInList(uint8_t index_start = 0) const;

    bool poll(void);

    bool sendBit(bool value);                                    // returns 1 if error occurred
    bool send(uint8_t dataByte);                                 // returns 1 if error occurred
    bool send(const uint8_t address[], uint8_t data_length = 1); // returns 1 if error occurred
    bool send(const uint8_t address[], uint8_t data_length,
              uint16_t &crc16); // returns 1 if error occurred
    // CRC takes ~7.4µs/byte (Atmega328P@16MHz) but is distributing the load between each bit-send to 0.9 µs/bit (see debug-crc-comparison.ino)
    // important: the final crc is expected to be inverted (crc=~crc) !!!

    bool recvBit(void);
    bool recv(uint8_t address[], uint8_t data_length = 1); // returns 1 if error occurred
    bool recv(uint8_t address[], uint8_t data_length,
              uint16_t &crc16); // returns 1 if error occurred

    timeOW_t waitLoopsCalibrate(void); // returns Instructions per loop
    void     waitLoops1ms(void);
    void     waitLoopsDebug(void) const;

    // mostly for debug, partly for state-machine handling
    void  printError(void) const;
    Error getError(void) const; // returns Error
    bool  hasError(void) const; // returns true if Error occurred
    void  raiseDeviceError(uint8_t cmd = 0);
    Error clearError(void);
};

#endif
