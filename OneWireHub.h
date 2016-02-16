#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <inttypes.h>

const bool dbg_CALCK    = 0; // give debug messages
const bool dbg_SEARCH   = 0; // give debug messages
const bool dbg_MATCHROM = 0; // give debug messages
const bool dbg_HINT     = 0; // give debug messages for called unimplemented functions of sensors

#define FALSE 0
#define TRUE  1

class OneWireItem;

class OneWireHub
{
private:

    static const uint8_t  ONEWIRESLAVE_COUNT                = 8;
    static const uint16_t ONEWIREIDMAP_COUNT                = 256;

    static const uint8_t ONEWIRE_NO_ERROR                   = 0;
    static const uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT      = 1;
    static const uint8_t ONEWIRE_WRITE_TIMESLOT_TIMEOUT     = 2;
    static const uint8_t ONEWIRE_WAIT_RESET_TIMEOUT         = 3;
    static const uint8_t ONEWIRE_VERY_LONG_RESET            = 4;
    static const uint8_t ONEWIRE_VERY_SHORT_RESET           = 5;
    static const uint8_t ONEWIRE_PRESENCE_LOW_ON_LINE       = 6;
    static const uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW  = 7;
    static const uint8_t ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH = 8;

    uint8_t pin_bitmask;
    uint8_t slave_count;
    uint8_t errno;
    volatile uint8_t *baseReg;

    uint8_t bits[ONEWIREIDMAP_COUNT];
    uint8_t idmap0[ONEWIREIDMAP_COUNT];
    uint8_t idmap1[ONEWIREIDMAP_COUNT];

    OneWireItem *elms[ONEWIRESLAVE_COUNT];  // make it private (use attach/detach)

    OneWireItem *SelectElm;

    bool recvAndProcessCmd();

    uint8_t waitTimeSlot();

    uint8_t waitTimeSlotRead();

    //int AnalizIds(uint8_t Pos, uint8_t BN, uint8_t BM, uint8_t mask);

public:

    explicit OneWireHub(uint8_t pin);

    uint8_t attach(OneWireItem &sensor);
    bool    detach(const OneWireItem &sensor);
    bool    detach(const uint8_t slave_number);

    int calck_mask(void);

    bool waitForRequest(const bool ignore_errors = false);

    bool waitReset(uint16_t timeout_ms);

    bool waitReset(void);

    bool presence(const uint8_t delta_us = 25);

    bool search(void);

    uint8_t sendData(const uint8_t buf[], const uint8_t data_len);

    uint8_t recvData(uint8_t buf[], const uint8_t data_len);

    void send(const uint8_t v);

    uint8_t recv(void);

    void sendBit(const uint8_t v);

    uint8_t recvBit(void);

    bool error(void)
    {
        if (errno == ONEWIRE_NO_ERROR) return 0;
        return 1;
    };

};

class OneWireItem
{
public:
    OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t ID[8];

    virtual bool duty(OneWireHub *hub) = 0;

    static uint8_t crc8(uint8_t addr[], uint8_t len);

    static uint16_t crc16(const uint8_t addr[], const uint8_t len);
};

void ow_crc16_reset(void);

void ow_crc16_update(uint8_t b);

uint16_t ow_crc16_get(void);
