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

// You can exclude CRC checks altogether by defining this to 0
#ifndef ONEWIRESLAVE_CRC
#define ONEWIRESLAVE_CRC 1
#endif

#ifndef ONEWIRESLAVE_CRC8_TABLE
#define ONEWIRESLAVE_CRC8_TABLE 0
#endif

static const uint8_t ONEWIRESLAVE_COUNT = 8;
static const int ONEWIREIDMAP_COUNT = 256;

#define FALSE 0
#define TRUE  1

#define ONEWIRE_NO_ERROR 0
#define ONEWIRE_READ_TIMESLOT_TIMEOUT 1
#define ONEWIRE_WRITE_TIMESLOT_TIMEOUT 2
#define ONEWIRE_WAIT_RESET_TIMEOUT 3
#define ONEWIRE_VERY_LONG_RESET 4
#define ONEWIRE_VERY_SHORT_RESET 5
#define ONEWIRE_PRESENCE_LOW_ON_LINE 6
#define ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW 7
#define ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH 8

class OneWireItem;

class OneWireHub
{
private:
    uint8_t pin_bitmask;
    volatile uint8_t *baseReg;

    uint8_t bits[ONEWIREIDMAP_COUNT];
    uint8_t idmap0[ONEWIREIDMAP_COUNT];
    uint8_t idmap1[ONEWIREIDMAP_COUNT];

    OneWireItem *SelectElm;

    bool recvAndProcessCmd();

    uint8_t waitTimeSlot();

    uint8_t waitTimeSlotRead();

    int AnalizIds(uint8_t Pos, uint8_t BN, uint8_t BM, uint8_t mask);

public:
    OneWireHub(uint8_t pin);

    OneWireItem *elms[ONEWIRESLAVE_COUNT];

    int calck_mask(void);

    bool waitForRequest(bool ignore_errors);

    bool waitReset(uint16_t timeout_ms);

    bool waitReset(void);

    bool presence(uint8_t delta);

    bool presence(void);

    bool search(void);

    uint8_t sendData(uint8_t buf[], uint8_t data_len);

    uint8_t recvData(uint8_t buf[], uint8_t data_len);

    void send(uint8_t v);

    uint8_t recv(void);

    void sendBit(uint8_t v);

    uint8_t recvBit(void);

    uint8_t errno;
};

class OneWireItem
{
public:
    OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    uint8_t ID[8];

    virtual bool duty(OneWireHub *hub) = 0;

#if ONEWIRESLAVE_CRC

    static uint8_t crc8(uint8_t addr[], uint8_t len);

    static uint16_t crc16(uint8_t addr[], uint8_t len);

#endif
};

void ow_crc16_reset(void);

void ow_crc16_update(uint8_t b);

uint16_t ow_crc16_get(void);
