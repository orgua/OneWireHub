// large parts are derived from https://github.com/MarkusLange/OneWireSlave/blob/master/OneWireSlave.cpp

#include "OneWireHub.h"
#include "OneWireItem.h"

OneWireHub::OneWireHub(uint8_t pin)
{
    _error = Error::NO_ERROR;

 	pin_bitMask = PIN_TO_BITMASK(pin);
	pin_baseReg = PIN_TO_BASEREG(pin);

    extend_timeslot_detection = 0;

    slave_count = 0;
    slave_selected = nullptr;

    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
        slave_list[i] = nullptr;

    // prepare pin
    volatile uint8_t *reg asm("r30") = pin_baseReg;
    DIRECT_MODE_INPUT(reg, pin_bitMask);

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

    if (position == 255)
        return 255;

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
    if (slave_list[slave_number] == nullptr)    return 0;
    if (!slave_count)                           return 0;
    if (slave_number >= ONEWIRESLAVE_LIMIT)     return 0;

    slave_list[slave_number] = nullptr;
    slave_count--;
    buildIDTree();
    return 1;
};


// just look through each bit of each ID and build a tree, so there are n=slaveCount decision-points
// trade-off: more online calculation, but @4Slave 16byte storage instead of 3*256 byte
uint8_t OneWireHub::getNrOfFirstBitSet(const mask_t mask)
{
    mask_t _mask = mask;
    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
    {
        if (_mask & 1)  return i;
        _mask >>= 1;
    }
    return 0;
}

// gone through the address, store this result
uint8_t OneWireHub::getNrOfFirstFreeIDTreeElement(void)
{
    for (uint8_t i = 0; i < ONEWIRE_TREE_SIZE; ++i)
        if (idTree[i].id_position == 255)
            return i;
    return 0;
};

// initial FN to build the ID-Tree
uint8_t OneWireHub::buildIDTree(void)
{
    mask_t mask_slaves = 0;
    mask_t bit_mask    = 0x01;

    // build mask
    for (uint8_t i = 0; i< ONEWIRESLAVE_LIMIT; ++i)
    {
        if (slave_list[i] != nullptr) mask_slaves |= bit_mask;
        bit_mask <<= 1;
    }

    for (uint8_t i = 0; i< ONEWIRE_TREE_SIZE; ++i)
        idTree[i].id_position    = 255;

    // begin with root-element
    buildIDTree(0, mask_slaves); // goto branch

    return 0;
}

// returns the branch that this iteration has worked on
uint8_t OneWireHub::buildIDTree(uint8_t position_IDBit, const mask_t mask_slaves)
{
    if (!mask_slaves) return (255);

    while (position_IDBit < 64)
    {
        mask_t mask_pos = 0;
        mask_t mask_neg = 0;
        const uint8_t pos_byte = (position_IDBit >> 3);
        const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));
        mask_t mask_id = 1;

        // search through all active slaves
        for (uint8_t id = 0; id < ONEWIRESLAVE_LIMIT; ++id)
        {
            if (mask_slaves & mask_id)
            {
                // if slave is in mask differentiate the bitValue
                if (slave_list[id]->ID[pos_byte] & mask_bit)
                    mask_pos |= mask_id;
                else
                    mask_neg |= mask_id;
            }
            mask_id <<= 1;
        }

        if (mask_neg && mask_pos)
        {
            // there was found a junction
            const uint8_t active_element = getNrOfFirstFreeIDTreeElement();

            idTree[active_element].id_position     = position_IDBit;
            idTree[active_element].slave_selected  = getNrOfFirstBitSet(mask_slaves);
            position_IDBit++;
            idTree[active_element].got_one         = buildIDTree(position_IDBit, mask_pos);
            idTree[active_element].got_zero        = buildIDTree(position_IDBit, mask_neg);
            return active_element;
        };

        position_IDBit++;
    }

    // gone through the address, store this result
    uint8_t active_element = getNrOfFirstFreeIDTreeElement();

    idTree[active_element].id_position     = 128;
    idTree[active_element].slave_selected  = getNrOfFirstBitSet(mask_slaves);
    idTree[active_element].got_one         = 255;
    idTree[active_element].got_zero        = 255;

    return active_element;
}

// legacy code
bool OneWireHub::waitForRequest(const bool ignore_errors)
{
    while (1)
    {
        bool interaction  = poll();

        if ((_error == Error::NO_ERROR) || ignore_errors)
        {
            continue;
        }
        else if (interaction)
        {
            return true;
        }
        if (_error != Error::NO_ERROR)
        {
            printError();
        }
    }
}


bool OneWireHub::poll(void)
{
    _error = Error::NO_ERROR;

    while (1)
    {
        // this additional check prevents an infinite loop when calling this FN without sensors attached
        if (!slave_count)           return true;

        //Once reset is done, go to next step
        if (!checkReset(10000))     return false; // TODO: should optimize, lower value is better

        // Reset is complete, tell the master we are present
        if (!showPresence())        return false;

        //Now that the master should know we are here, we will get a command from the bus
        if (!recvAndProcessCmd())   return false;

        // on total success we want to start again, because the next reset could only be ~125 us away
    }
}



bool OneWireHub::checkReset(uint16_t timeout_us) // there is a specific high-time needed before a reset may occur -->  >120us
{
    volatile uint8_t *reg asm("r30") = pin_baseReg;

    noInterrupts();
    DIRECT_MODE_INPUT(reg, pin_bitMask);
    interrupts();

    wait(ONEWIRE_TIME_BUS_CHANGE_MAX); // let the input settle

    // is entered if there are two resets within a given time (timeslot-detection can issue this skip)
    if (skip_reset_detection)
    {
        skip_reset_detection = 0;
        _error = Error::NO_ERROR;

        if (!waitWhilePinIs(0, ONEWIRE_TIME_RESET_MIN - ONEWIRE_TIME_SLOT_MAX))
        {
            return true;
        }
    }

    if (!DIRECT_READ(reg, pin_bitMask)) return false; // just leave if pin is Low, don't bother to wait

    // wait for the bus to become low (master-controlled), since we are polling we don't know for how long it was zero
    if (!waitWhilePinIs(1, timeout_us))
    {
        //_error = Error::WAIT_RESET_TIMEOUT;
        return false;
    }

    uint32_t time_start = micros();

    // wait for bus-release by master
    if (!waitWhilePinIs(0, ONEWIRE_TIME_RESET_MAX))
    {
        _error = Error::VERY_LONG_RESET;
        return false;
    }

    // If the master pulled low for to short this will trigger an error
    if ((time_start + ONEWIRE_TIME_RESET_MIN) > micros())
    {
        //_error = Error::VERY_SHORT_RESET;
        return false;
    }

    return true;
}


bool OneWireHub::showPresence(void)
{
    volatile uint8_t *reg asm("r30") = pin_baseReg;

    // Master will delay it's "Presence" check (bus-read)  after the reset
    waitWhilePinIs( 1, ONEWIRE_TIME_PRESENCE_SAMPLE_MIN); // no pinCheck demanded, but this additional check can cut waitTime

    // pull the bus low and hold it some time
    noInterrupts();
    DIRECT_WRITE_LOW(reg, pin_bitMask);
    DIRECT_MODE_OUTPUT(reg, pin_bitMask);    // drive output low
    interrupts();

    wait(ONEWIRE_TIME_PRESENCE_LOW_STD);

    noInterrupts();
    DIRECT_MODE_INPUT(reg, pin_bitMask);     // allow it to float
    interrupts();

    // When the master or other slaves release the bus within a given time everything is fine
    if (!waitWhilePinIs( 0, (ONEWIRE_TIME_PRESENCE_LOW_MAX - ONEWIRE_TIME_PRESENCE_LOW_STD)))
    {
        _error = Error::PRESENCE_LOW_ON_LINE;
        return false;
    }

    extend_timeslot_detection = 1; // DS9490R takes 7-9 ms after presence-detection to start with timeslots
    return true;
}

void OneWireHub::extendTimeslot(void)
{
    extend_timeslot_detection = 1;
}


bool OneWireHub::search(void)
{
    uint8_t position_IDBit  = 0;
    uint8_t trigger_pos     = 0;
    uint8_t active_slave    = idTree[trigger_pos].slave_selected;
    uint8_t trigger_bit     = idTree[trigger_pos].id_position;

    while (position_IDBit < 64)
    {
        // if junction is reached, act different
        if (position_IDBit == trigger_bit)
        {
            sendBit(false);
            sendBit(false);
            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR) return false;

            // switch to next junction
            if (bit_recv)   trigger_pos = idTree[trigger_pos].got_one;
            else            trigger_pos = idTree[trigger_pos].got_zero;

            active_slave = idTree[trigger_pos].slave_selected;

            if (trigger_pos == 255)
                trigger_bit  = 255;
            else
                trigger_bit  = idTree[trigger_pos].id_position;

        }
        else
        {
            const uint8_t pos_byte = (position_IDBit >> 3);
            const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));
            bool bit_send;

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

            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR)  return false;

            if (bit_send != bit_recv)
                return false;
        }
        position_IDBit++;
    }

    slave_selected = slave_list[active_slave];
    return true;
}

bool OneWireHub::recvAndProcessCmd(void)
{
    uint8_t address[8];
    bool    flag = false;
    uint8_t cmd = recv();
    if (skip_reset_detection)       return true; // stay in loop and trigger another datastream-detection
    if (_error != Error::NO_ERROR)  return false;

    switch (cmd)
    {
        case 0xF0: // Search rom
            search();
            return true; // always trigger a re-init after search

        case 0x55: // MATCH ROM - Choose/Select ROM
            recv(address, 8);
            if (_error != Error::NO_ERROR)  return false;

            slave_selected = nullptr;

            for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
            {
                if (slave_list[i] == nullptr) continue;

                flag = true;
                for (uint8_t j = 0; j < 8; ++j)
                {
                    if (slave_list[i]->ID[j] != address[j])
                    {
                        flag = false;
                        break;
                    }
                }

                if (flag)
                {
                    slave_selected = slave_list[i];
                    break;
                }
            }

            if (!flag)
            {
                return false;
            }

            if (slave_selected != nullptr)
            {
                extend_timeslot_detection = 1;
                slave_selected->duty(this);

            }
            return true;

        case 0xCC: // SKIP ROM
            slave_selected = nullptr;

            for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
            {
                if (slave_list[i] != nullptr)
                {
                    slave_selected = slave_list[i];
                    break;
                }
            }

            if (slave_selected != nullptr)
            {
                extend_timeslot_detection = 1;
                slave_selected->duty(this);
            }
            return true;

        case 0x33: // READ ROM
        case 0x0F: // OLD READ ROM
            // only usable when there is ONE slave on the bus

        default: // Unknown command
            _error = Error::INCORRECT_ONEWIRE_CMD;
            _error_cmd = cmd;
    }
    return false;
}

bool OneWireHub::send(const uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_sent = 0;

    for (bytes_sent; bytes_sent < data_length; ++bytes_sent)
    {
        send(address[bytes_sent]);
        if (_error != Error::NO_ERROR)  break;
    }
    return (bytes_sent == data_length);
}

bool OneWireHub::send(const uint8_t dataByte)
{
    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        sendBit((bitMask & dataByte) ? bool(1) : bool(0));
        if (_error != Error::NO_ERROR) return false;
    }
    return true;
}

uint16_t OneWireHub::sendAndCRC16(uint8_t dataByte, uint16_t crc16)
{
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        sendBit((0x01 & dataByte) ? bool(1) : bool(0));

        uint8_t mix = ((uint8_t) crc16 ^ dataByte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        dataByte >>= 1;

        if (_error != Error::NO_ERROR) return 0; // CRC is not important if sending fails
    }
    return crc16;
}

bool OneWireHub::sendBit(const bool value)
{
    // wait for a low to high transition followed by a high to low within the time-out
    if (!awaitTimeSlotAndWrite(!value))
    {
        _error = Error::WRITE_TIMESLOT_TIMEOUT;
        return false; // timeslot violation
    }

    if (value)  waitWhilePinIs( 0, ONEWIRE_TIME_READ_ONE_LOW_MAX); // no pinCheck demanded, but this additional check can cut waitTime
    else
    {
        // if we wait for release we could detect faulty writing slots --> pedantic Mode not needed for now
        wait(ONEWIRE_TIME_WRITE_ZERO_LOW_STD);
        volatile uint8_t *reg asm("r30") = pin_baseReg;
        DIRECT_MODE_INPUT(reg, pin_bitMask);
    }

    return true;
}


bool OneWireHub::recv(uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_received = 0;

    for (bytes_received; bytes_received < data_length; ++bytes_received)
    {
        address[bytes_received] = recv();
        if (_error != Error::NO_ERROR)  break;
    }
    return (bytes_received == data_length);
}

uint8_t OneWireHub::recv(void)
{
    uint8_t value = 0;

    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (recvBit())  value |= bitMask;
        if (_error != Error::NO_ERROR)  break;
    }
    return value;
}

// TODO: not happy with the interface - call by ref is slow here. maybe use a crc in class and expand with crc-reset and get?
uint8_t OneWireHub::recvAndCRC16(uint16_t &crc16)
{
    uint8_t value = 0;
    uint8_t mix = 0;

    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (recvBit())
        {
            value |= bitMask;
            mix = 1;
        }
        else mix = 0;

        mix ^= static_cast<uint8_t>(crc16) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);

        if (_error != Error::NO_ERROR) return 0;
    }
    return value;
}

bool OneWireHub::recvBit(void)
{

    // wait for a low to high transition followed by a high to low within the time-out
    if (!awaitTimeSlotAndWrite())
    {
        if (extend_timeslot_detection==2)
        {
            _error = Error::FIRST_TIMESLOT_TIMEOUT;
            extend_timeslot_detection = 0;
        }
        else
        {
            _error = Error::READ_TIMESLOT_TIMEOUT;
        }

        return 0;
    }

    waitWhilePinIs( 0, ONEWIRE_TIME_READ_STD); // no pinCheck demanded, but this additional check can cut waitTime
    volatile uint8_t *reg asm("r30") = pin_baseReg;
    DIRECT_MODE_INPUT(reg, pin_bitMask);

    return DIRECT_READ(reg, pin_bitMask);
}

#define USE_DELAY 1

void OneWireHub::wait(const uint16_t timeout_us)
{
#if (USE_DELAY > 0)
    delayMicroseconds(timeout_us);
#else
    uint32_t time_trigger = micros() + timeout_us;
    while (micros() < time_trigger)
    {
    }
#endif
}

#define USE_MICROS 1

bool OneWireHub::waitWhilePinIs(const bool value, const uint16_t timeout_us)
{
    volatile uint8_t *reg asm("r30") = pin_baseReg;
    if (DIRECT_READ(reg, pin_bitMask) != value) return true; // shortcut

#if (USE_MICROS > 0)
    uint32_t time_trigger = micros() + timeout_us;
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (micros() >= time_trigger) return false;
    }
#else
    uint16_t retries = static_cast<uint16_t>(microsecondsToClockCycles(timeout_us)/11);
    while (DIRECT_READ(reg, pin_bitMask) == value)
    {
        if (--retries == 0) return false;
    }
#endif
    return true;
}


#define NEW_WAIT 0 // TODO: NewWait does not work as expected (and is deprecated)
#if (NEW_WAIT > 0)

// wait for a low to high transition followed by a high to low within the time-out
bool OneWireHub::awaitTimeSlotAndWrite(void)
{
    noInterrupts();
    volatile uint8_t *reg asm("r30") = pin_baseReg;
    DIRECT_MODE_INPUT(reg, pin_bitMask);
    interrupts();

    //While bus is low, retry until HIGH

    if (!waitWhilePinIs( false, ONEWIRE_TIME_SLOT_MAX))
    {
        _error = Error::READ_TIMESLOT_TIMEOUT_LOW;
        return false;
    }

    uint16_t wait_us;
    if (extend_timeslot_detection)
    {
        //extend_timeslot_detection = 0;
        wait_us = ONEWIRE_TIME_PRESENCE_HIGH_MAX;
    }
    else
    {
        wait_us = ONEWIRE_TIME_SLOT_MAX;
    }

    if (!waitWhilePinIs( true, wait_us ))
    {
        _error = Error::READ_TIMESLOT_TIMEOUT_HIGH;
        return false;
    }

    return true;
}

#else

#define TIMESLOT_WAIT_RETRY_COUNT  static_cast<uint16_t>(microsecondsToClockCycles(135)/8)   /// :11 is a specif value for 8bit-atmega, still to determine
bool OneWireHub::awaitTimeSlotAndWrite(const bool writeZero)
{
    volatile uint8_t *reg asm("r30") = pin_baseReg;
    noInterrupts();
    DIRECT_WRITE_LOW(reg, pin_bitMask);
    DIRECT_MODE_INPUT(reg, pin_bitMask);

    //While bus is low, retry until HIGH
    uint16_t retries = TIMESLOT_WAIT_RETRY_COUNT;
    while (!DIRECT_READ(reg, pin_bitMask))
    {
        if (--retries == 0)
        {
            if (extend_timeslot_detection==2)
            {
                extend_timeslot_detection = 0;
                skip_reset_detection      = 1;
            }
            else
            {
                _error = Error::READ_TIMESLOT_TIMEOUT_LOW;
            }
            interrupts();
            return false;
        }
    }

    // extend the wait-time after reset and presence-detection
    if (extend_timeslot_detection == 1)
    {
        retries = 65535;
        extend_timeslot_detection = 2; // prepare to detect missing timeslot or second reset
    }
    else
    {
        //retries = TIMESLOT_WAIT_RETRY_COUNT;
        retries = 65535; // TODO: workaround for better compatibility (will be solved later)
    }

    //Wait for bus to fall form 1 to 0
    while (DIRECT_READ(reg, pin_bitMask))
    {
        if (--retries == 0)
        {
            _error = Error::READ_TIMESLOT_TIMEOUT_HIGH;
            interrupts();
            return false;
        }
    }

    if (writeZero)
    {
        // Low is allready set
        DIRECT_MODE_OUTPUT(reg, pin_bitMask);
    }

    interrupts();
    return true;
}
#endif


void OneWireHub::printError(void)
{
#if USE_SERIAL_DEBUG
     if (_error == Error::NO_ERROR)                        return;
     Serial.print("Error: ");
     if      (_error == Error::READ_TIMESLOT_TIMEOUT)      Serial.print("read timeslot timeout");
     else if (_error == Error::WRITE_TIMESLOT_TIMEOUT)     Serial.print("write timeslot timeout");
     else if (_error == Error::WAIT_RESET_TIMEOUT)         Serial.print("reset wait timeout");
     else if (_error == Error::VERY_LONG_RESET)            Serial.print("very long reset");
     else if (_error == Error::VERY_SHORT_RESET)           Serial.print("very short reset");
     else if (_error == Error::PRESENCE_LOW_ON_LINE)       Serial.print("presence low on line");
     else if (_error == Error::READ_TIMESLOT_TIMEOUT_LOW)  Serial.print("read timeout low");
     else if (_error == Error::READ_TIMESLOT_TIMEOUT_HIGH) Serial.print("read timeout high");
     else if (_error == Error::PRESENCE_HIGH_ON_LINE)      Serial.print("presence high on line");
     else if (_error == Error::INCORRECT_ONEWIRE_CMD)      Serial.print("incorrect onewire command");
     else if (_error == Error::INCORRECT_SLAVE_USAGE)      Serial.print("slave was used in incorrect way");
     else if (_error == Error::TRIED_INCORRECT_WRITE)      Serial.print("tried to write in read-slot");
     else if (_error == Error::FIRST_TIMESLOT_TIMEOUT)     Serial.print("found no timeslot after reset / presence (is OK)");

    if ((_error == Error::INCORRECT_ONEWIRE_CMD)||(_error == Error::INCORRECT_SLAVE_USAGE))
    {
        Serial.print(" [0x");
        Serial.print(_error_cmd,HEX);
        Serial.println("]");
    }
    else
    {
        Serial.println("");
    }

#endif
}

bool OneWireHub::getError(void)
{
    return (_error != Error::NO_ERROR);
};

void OneWireHub::raiseSlaveError(const uint8_t cmd)
{
    _error = Error::INCORRECT_SLAVE_USAGE;
    _error_cmd = cmd;
};
