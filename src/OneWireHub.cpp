// large parts are derived from https://github.com/MarkusLange/OneWireSlave/blob/master/OneWireSlave.cpp

#include "OneWireHub.h"
#include "OneWireItem.h"
#include "pins_arduino.h"

/*
extern "C" {
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
}
*/

#define DIRECT_READ(base, mask)        (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)  ((*(base+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask) ((*(base+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)   ((*(base+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)  ((*(base+2)) |= (mask))


OneWireHub::OneWireHub(uint8_t pin)
{
    _error = ONEWIRE_NO_ERROR;

    pin_bitmask = digitalPinToBitMask(pin);

    baseReg = portInputRegister(digitalPinToPort(pin)); // TODO: remove other instances

    //bits[ONEWIREIDMAP_COUNT]; // TODO: init
    //idmap0[ONEWIREIDMAP_COUNT];
    //idmap1[ONEWIREIDMAP_COUNT];

    slave_count = 0;
    slave_selected = nullptr;

    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
        slave_list[i] = nullptr;

};


// attach a sensor to the hub
uint8_t OneWireHub::attach(OneWireItem &sensor)
{
    if (slave_count >= ONEWIRESLAVE_LIMIT) return 0; // hub is full

    // find position of next free storage-position
    uint8_t position = 255;
    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
    {
        // check for already attached sensors
        if (slave_list[i] == &sensor)
            return i;

        // store position of first empty space
        if ((position>ONEWIRESLAVE_LIMIT) && (slave_list[i] == nullptr))
        {
            position = i;
            break;
        }
    }

    slave_list[position] = &sensor;
    slave_count++;
    buildIDTree();
    return position;
};

bool    OneWireHub::detach(const OneWireItem &sensor)
{
    // find position of sensor
    uint8_t position = 255;
    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
    {
        if (slave_list[i] == &sensor)
        {
            position = i;
            break;
        }
    }

    if (position != 255)    return detach(position);
    else                    return 0;
};

bool    OneWireHub::detach(const uint8_t slave_number)
{
    if (slave_list[slave_number] == nullptr)          return 0;
    if (!slave_count)                           return 0;
    if (slave_number >= ONEWIRESLAVE_LIMIT)     return 0;

    slave_list[slave_number] = nullptr;
    slave_count--;
    buildIDTree();
    return 1;
};


// just look through each bit of each ID and build a tree, so there are n=slavecount decision-points
// tradeoff: more online calculation, but @4Slave 16byte storage instead of 3*256 byte
uint8_t OneWireHub::getNrOfFirstBitSet(const uint8_t mask)
{
    uint8_t _mask = mask;
    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
    {
        if (_mask & 1)  return i;
        _mask >>= 1;
    }
}

// initial FN to build the ID-Tree
uint8_t OneWireHub::buildIDTree(void)
{
    uint8_t mask_slaves = 0;

    for (uint8_t i = 0; i< ONEWIRESLAVE_LIMIT; ++i)
        if (slave_list[i] != nullptr) mask_slaves |= (1 << i);

    for (uint8_t i = 0; i< ONEWIRETREE_SIZE; ++i)
        idTree[i].idPosition    = 255;

    // begin with root-element
    buildIDTree(0, mask_slaves); // goto branch

    if (dbg_IDTREE)
    {
        Serial.println("Calculate idTree: ");
        for (uint8_t i = 0; i < ONEWIRETREE_SIZE; ++i)
        {

            Serial.print("Slave: ");
            if (idTree[i].slave_selected < 10) Serial.print("  ");
            Serial.print(idTree[i].slave_selected);
            Serial.print(" bitPos: ");
            if (idTree[i].idPosition < 10) Serial.print(" ");
            if (idTree[i].idPosition < 100) Serial.print(" ");
            Serial.print(idTree[i].idPosition);
            Serial.print(" if0gt: ");
            if (idTree[i].gotZero < 10) Serial.print(" ");
            if (idTree[i].gotZero < 100) Serial.print(" ");
            Serial.print(idTree[i].gotZero);
            Serial.print(" if1gt: ");
            if (idTree[i].gotOne < 10) Serial.print(" ");
            if (idTree[i].gotOne < 100) Serial.print(" ");
            Serial.println(idTree[i].gotOne);
        }
    }
    return 0;
}

// returns the branch that this iteration has worked on
uint8_t OneWireHub::buildIDTree(uint8_t position_IDBit, const uint8_t mask_slaves)
{
    if (!mask_slaves) return (255);

    while (position_IDBit < 64)
    {
        uint8_t mask_pos = 0;
        uint8_t mask_neg = 0;
        const uint8_t pos_byte = (position_IDBit >> 3);
        const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));

        // search through all active slaves
        for (uint8_t id = 0; id < ONEWIRESLAVE_LIMIT; ++id)
        {
            const uint8_t mask_id = (static_cast<uint8_t>(1) << id);

            if (mask_slaves & mask_id)
            {
                // if slave is in mask differentiate the bitvalue
                if (slave_list[id]->ID[pos_byte] & mask_bit)
                    mask_pos |= mask_id;
                else
                    mask_neg |= mask_id;
            }
        }

        if (mask_neg && mask_pos)
        {
            // there was found a junction
            uint8_t active_element = 0;
            for (uint8_t i = 0; i < ONEWIRETREE_SIZE; ++i)
            {
                if (idTree[i].idPosition == 255)
                {
                    active_element = i;
                    break;
                }
            };

            idTree[active_element].idPosition    = position_IDBit;
            idTree[active_element].slave_selected = getNrOfFirstBitSet(mask_slaves);
            position_IDBit++;
            idTree[active_element].gotOne         = buildIDTree(position_IDBit, mask_pos);
            idTree[active_element].gotZero        = buildIDTree(position_IDBit, mask_neg);
            return active_element;
        };

        position_IDBit++;
    }

    // gone through the adress, store this result
    // TODO: code duplication
    uint8_t active_element = 0;
    for (uint8_t i = 0; i < ONEWIRETREE_SIZE; ++i)
    {
        if (idTree[i].idPosition == 255)
        {
            active_element = i;
            break;
        }
    };
    idTree[active_element].idPosition     = 128;
    idTree[active_element].slave_selected = getNrOfFirstBitSet(mask_slaves);
    idTree[active_element].gotOne         = 255;
    idTree[active_element].gotZero        = 255;

    return active_element;
}


bool OneWireHub::waitForRequest(const bool ignore_errors) // TODO: maybe build a non blocking version of this? and more common would be inverse of ignore_errors (=blocking)
{
    _error = ONEWIRE_NO_ERROR;

    while (1)
    {
        bool interaction  = poll();

        if ((_error == ONEWIRE_NO_ERROR) || ignore_errors)
        {
            continue;
        }
        else if (interaction)
        {
            return true;
        }
        if (dbg_HINT && _error)
        {
            printError();
        }
    }
}


bool OneWireHub::poll(void)
{
    // this additional check prevents an infinite loop when calling this FN without sensors attached
    if (!slave_count)           return true;

    //Once reset is done, go to next step
    if (!checkReset(2))         return false;

    // Reset is complete, tell the master we are prsent
    if (!showPresence())        return false;

    //Now that the master should know we are here, we will get a command from the bus
    if (recvAndProcessCmd())    return true;
    else                        return false;
}


bool OneWireHub::checkReset(uint16_t timeout_us)
{
    volatile uint8_t *reg asm("r30") = baseReg;

    cli();
    DIRECT_MODE_INPUT(reg, pin_bitmask);
    sei();

    // looks if bus is low, since we are polling we don't know for how long it was zero
    bool      bus_was_high = 0;
    uint32_t  time_trigger = micros() + timeout_us;
    _error = ONEWIRE_NO_ERROR;
    delayMicroseconds(ONEWIRE_TIME_BUS_CHANGE_MAX); // let the input settle
    while (DIRECT_READ(reg, pin_bitmask)) // TODO: replace this with IRQ?
    {
        if (micros() > time_trigger)    return false;
        // if reached this point (bus was high), this could be an indicator for a sleep after bus goes low
        bus_was_high = 1;
    }

    // Set to wait for bus=high by master
    time_trigger = micros() + ONEWIRE_TIME_RESET_MAX;
    while (DIRECT_READ(reg, pin_bitmask) == 0)
    {
        if (micros() > time_trigger)
        {
            _error = ONEWIRE_VERY_LONG_RESET;
            return false;
        }
    }

    // If the master pulled low for to short this will trigger an error
    if (bus_was_high && ((time_trigger - ONEWIRE_TIME_RESET_MAX + ONEWIRE_TIME_RESET_MIN) > micros()))
    {
        _error = ONEWIRE_VERY_SHORT_RESET;
        return false;
    }

    return true;
}


bool OneWireHub::showPresence(void)
{
    // Master will now delay before it's "Presence" check
    delayMicroseconds(ONEWIRE_TIME_PRESENCE_HIGH_STD);

    uint32_t    time_trigger = micros() + ONEWIRE_TIME_PRESENCE_HIGH_MAX;

    volatile uint8_t *reg asm("r30") = baseReg;

    // pull the bus low and hold it some time
    cli();
    DIRECT_WRITE_LOW(reg, pin_bitmask);
    DIRECT_MODE_OUTPUT(reg, pin_bitmask);    // drive output low
    sei();

    delayMicroseconds(ONEWIRE_TIME_PRESENCE_LOW_STD);

    cli();
    DIRECT_MODE_INPUT(reg, pin_bitmask);     // allow it to float
    sei();

    // When the master pulls the bus high within a given time everything is fine
    while (!DIRECT_READ(reg, pin_bitmask))
    {
        if (micros() > time_trigger)
        {
            _error = ONEWIRE_VERY_LONG_RESET;
            return false;
        }

    }

    _error = ONEWIRE_NO_ERROR;
    return true;
}


bool OneWireHub::search(void)
{
    uint8_t position_IDBit = 0;

    uint8_t trigger_pos  = 0;
    uint8_t active_slave = idTree[trigger_pos].slave_selected;
    uint8_t trigger_bit  = idTree[trigger_pos].idPosition;

    while (position_IDBit < 64)
    {
        // if junction is reached, act different
        if (position_IDBit == trigger_bit)
        {
            sendBit(false);
            sendBit(false);
            uint8_t bit_recv = recvBit();
            if (_error != ONEWIRE_NO_ERROR)
                return false;
            // switch to next junction
            if (bit_recv)   trigger_pos = idTree[trigger_pos].gotOne;
            else            trigger_pos = idTree[trigger_pos].gotZero;
            active_slave = idTree[trigger_pos].slave_selected;
            if (trigger_pos == 255)
                trigger_bit  = 255;
            else
                trigger_bit  = idTree[trigger_pos].idPosition;

        }
        else
        {
            const uint8_t pos_byte = (position_IDBit >> 3);
            const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));
            uint8_t bit_send, bit_recv;

            if (slave_list[active_slave]->ID[pos_byte] & mask_bit)
            {
                bit_send = 1;
                sendBit(true);
                sendBit(false);
            }
            else
            {
                bit_send = 0;
                sendBit(false);
                sendBit(true);
            }

            bit_recv = recvBit();
            if (_error != ONEWIRE_NO_ERROR)
                return false;

            if (bit_send != bit_recv)
                return false;
        }
        position_IDBit++;
    }

    if (dbg_SEARCH)
    {
        Serial.print("Found:");
        Serial.println(active_slave);
    }

    slave_selected = slave_list[active_slave];
    return true;
}

bool OneWireHub::recvAndProcessCmd(void)
{
    uint8_t addr[8];
    bool flag;

    uint8_t cmd = recv();
    // TODO: removed while(1) loop
    switch (cmd)
    {
        case 0xF0: // Search rom
            search();
            //delayMicroseconds(1100); // sweetspot (to low - no data/PL; to high - no ds2401 without data) --> <1200, >1000, was 6900
            return true; // always trigger a re-init after search

        case 0x55: // MATCH ROM - Choose/Select ROM
            recv(addr, 8);
            if (_error != ONEWIRE_NO_ERROR)
                return false;

            flag = false;
            slave_selected = 0;

            for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
            {
                if (slave_list[i] == nullptr) continue;

                flag = true;
                for (uint8_t j = 0; j < 8; ++j)
                {
                    if (slave_list[i]->ID[j] != addr[j])
                    {
                        flag = false;
                        break;
                    }
                }

                if (flag)
                {
                    slave_selected = slave_list[i];

                    if (dbg_MATCHROM)
                    {
                        Serial.print("MATCH ROM=");
                        Serial.println(i);
                    }

                    break;
                }
            }

            if (!flag) return false;
            if (slave_selected != nullptr) slave_selected->duty(this);
            return true;


        case 0xCC: // SKIP ROM
            slave_selected = nullptr;
            return true;

        case 0x33: // READ ROM
        case 0x0F: // OLD READ ROM
            // only usable when there is ONE slave on the bus

        default: // Unknow command
            if (dbg_HINT)
            {
                Serial.print("U:");
                Serial.println(cmd, HEX);
            }
    }
    return false;
}

// TODO: there seems to be something wrong when first receiving and then sending, master has to wait a moment, otherwise it fails
bool OneWireHub::send(const uint8_t buf[], const uint8_t len)
{
    uint8_t bytes_sended = 0;

    for (uint8_t i = 0; i < len; ++i)
    {
        send(buf[i]);
        if (_error != ONEWIRE_NO_ERROR)
            break;
        bytes_sended++;
    }
    return bytes_sended; // TODO: resolve this issue
}

bool OneWireHub::send(const uint8_t databyte)
{
    _error = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask; bitmask <<= 1)
    {
        sendBit((bitmask & databyte) ? 1 : 0);
        if (_error) return false;
    }
    return true;
}


bool OneWireHub::sendBit(const uint8_t v)
{
    volatile uint8_t *reg asm("r30") = baseReg;

    cli();
    DIRECT_MODE_INPUT(reg, pin_bitmask);
    // wait for a low to high transition followed by a high to low within the time-out
    if (!waitTimeSlot())
    {
        sei();
        return false;
    }
    if (v & 1)  delayMicroseconds(32); // TODO: was 30 before
    else
    {
        cli();
        DIRECT_WRITE_LOW(reg, pin_bitmask);
        DIRECT_MODE_OUTPUT(reg, pin_bitmask);
        delayMicroseconds(32); // TODO: was 30 before
        DIRECT_WRITE_HIGH(reg, pin_bitmask);
    }
    sei();
    return true;
}

// CRC takes ~7.4µs/byte (Atmega328P@16MHz) but is distributing the load between each bit-send to 0.9 µs/bit (see debug-crc-comparison.ino)
// important: the final crc is expected to be inverted (crc=~crc) !!!
uint16_t OneWireHub::sendAndCRC16(uint8_t databyte, uint16_t crc16)
{
    _error = ONEWIRE_NO_ERROR;
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        sendBit((0x01 & databyte) ? 1 : 0);

        uint8_t mix = ((uint8_t) crc16 ^ databyte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        databyte >>= 1;

        if (_error) return false;
    }
    return crc16;
}


uint8_t OneWireHub::recv(uint8_t buf[], const uint8_t len)
{
    uint8_t bytes_received = 0;

    for (int i = 0; i < len; ++i)
    {
        buf[i] = recv();
        if (_error != ONEWIRE_NO_ERROR)
            break;
        bytes_received++;
    }
    return bytes_received; // TODO: resolve to true on success,
}

uint8_t OneWireHub::recv(void)
{
    uint8_t r = 0;

    _error = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask && (_error == ONEWIRE_NO_ERROR); bitmask <<= 1)
        if (recvBit())
            r |= bitmask;
    return r;
}

uint8_t OneWireHub::recvBit(void)
{
    volatile uint8_t *reg asm("r30") = baseReg;
    uint8_t r;

    cli();
    DIRECT_MODE_INPUT(reg, pin_bitmask);
    // wait for a low to high transition followed by a high to low within the time-out
    if (!waitTimeSlot())
    {
        sei();
        return false;
    }

    delayMicroseconds(30);
    r = DIRECT_READ(reg, pin_bitmask);
    sei();
    return r;
}

#define NEWWAIT 0 // TODO: does not work as expected
#if (NEWWAIT > 0)

// wait for a low to high transition followed by a high to low within the time-out
bool OneWireHub::waitTimeSlot(void)
{
    volatile uint8_t *reg asm("r30") = baseReg;

    //While bus is low, retry until HIGH
    uint32_t time_trigger = micros() + ONEWIRE_TIME_SLOT_MAX;
    while (!DIRECT_READ(reg, pin_bitmask))
    {
        if (micros() > time_trigger)
        {
            _error = ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW;
            return false;
        }
    }

    //Wait for bus to fall form 1 to 0
    time_trigger = micros() + ONEWIRE_TIME_SLOT_MAX;
    while (DIRECT_READ(reg, pin_bitmask))
    {
        if (micros() > time_trigger)
        {
            _error = ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH;
            return false;
        }
    }
    return true;
}

#else

#define TIMESLOT_WAIT_RETRY_COUNT  (microsecondsToClockCycles(135))
bool OneWireHub::waitTimeSlot(void)
{
    volatile uint8_t *reg asm("r30") = baseReg;

    //While bus is low, retry until HIGH
    uint16_t retries = TIMESLOT_WAIT_RETRY_COUNT;
    while (!DIRECT_READ(reg, pin_bitmask))
    {
        if (--retries == 0)
            return false;
    }

    //Wait for bus to fall form 1 to 0
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    while (DIRECT_READ(reg, pin_bitmask))
    {
        if (--retries == 0)
            return false;
    }
    return true;
}
#endif


void OneWireHub::printError(void)
{
 if (dbg_HINT)
 {
     if (_error == ONEWIRE_NO_ERROR)                        return;
     else if (_error == ONEWIRE_READ_TIMESLOT_TIMEOUT)      Serial.print("Err1: read timeslot timeout");
     else if (_error == ONEWIRE_WRITE_TIMESLOT_TIMEOUT)     Serial.print("Err2: write timeslot timeout");
     else if (_error == ONEWIRE_WAIT_RESET_TIMEOUT)         Serial.print("Err3: reset wait timeout");
     else if (_error == ONEWIRE_VERY_LONG_RESET)            Serial.print("Err4: very long reset");
     else if (_error == ONEWIRE_VERY_SHORT_RESET)           Serial.print("Err5: very short reset");
     else if (_error == ONEWIRE_PRESENCE_LOW_ON_LINE)       Serial.print("Err6: presence low on line");
     else if (_error == ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW)  Serial.print("Err7: read timeout low");
     else if (_error == ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH) Serial.print("Err8: read timeout high");
     Serial.println("");
 }
}
