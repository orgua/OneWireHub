// large parts are derived from https://github.com/MarkusLange/OneWireSlave/blob/master/OneWireSlave.cpp

#include "OneWireHub.h"
#include "OneWireItem.h"

OneWireHub::OneWireHub(const uint8_t pin, const uint8_t factor_ipl)
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
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    // debug:
#if USE_GPIO_DEBUG
    debug_bitMask = PIN_TO_BITMASK(GPIO_DEBUG_PIN);
    debug_baseReg = PIN_TO_BASEREG(GPIO_DEBUG_PIN);
    DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
    DIRECT_MODE_OUTPUT(debug_baseReg, debug_bitMask);
#endif

    // prepare timings
    if (factor_ipl == 1) // read back value from hub-config
    {
        static_assert(VALUE_IPL, "You tried to use fixed timing-values. Your architecture has not been calibrated yet, please run examples/debug/auto_timing and report instructions per loop (IPL) to https://github.com/orgua/OneWireHub");
        calibrate_loop_timing = false;
        factor_nspl = VALUE_IPL * VALUE1k / microsecondsToClockCycles(1); // nanoseconds per loop
        waitLoopsConfig();
    }
    else if (factor_ipl > 1)
    {
        calibrate_loop_timing = false;
        factor_nspl = factor_ipl * VALUE1k / microsecondsToClockCycles(1); // nanoseconds per loop
        waitLoopsConfig();
    }
    else
    {
        calibrate_loop_timing = true; // do an auto-calibrate during first attachment (should be avoided)
    }
};


// attach a sensor to the hub
uint8_t OneWireHub::attach(OneWireItem &sensor)
{
    if (slave_count >= ONEWIRESLAVE_LIMIT) return 0; // hub is full

    // prepare timing, done here because this FN is always called before hub is used
    if (calibrate_loop_timing)
    {
        waitLoopsCalibrate();
    };

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
};

// return next not empty element in slave-list
uint8_t OneWireHub::getIndexOfNextSensorInList(const uint8_t index_start = 0)
{
    for (uint8_t i = index_start; i < ONEWIRE_TREE_SIZE; ++i)
    {
        if (slave_list[i] != nullptr)  return i;
    }
    return 0;
};

// gone through the address, store this result
uint8_t OneWireHub::getNrOfFirstFreeIDTreeElement(void)
{
    for (uint8_t i = 0; i < ONEWIRE_TREE_SIZE; ++i)
    {
        if (idTree[i].id_position == 255) return i;
    }
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
    {
        idTree[i].id_position = 255;
    }

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
        if (!checkReset())     return false;

        // Reset is complete, tell the master we are present
        digitalWrite(GPIO_DEBUG_PIN,HIGH);
        if (!showPresence())        return false;
        digitalWrite(GPIO_DEBUG_PIN,LOW);

        //Now that the master should know we are here, we will get a command from the bus
        if (!recvAndProcessCmd())   return false;

        // on total success we want to start again, because the next reset could only be ~125 us away
    }
}



bool OneWireHub::checkReset(void) // there is a specific high-time needed before a reset may occur -->  >120us
{
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    wait(ONEWIRE_TIME_BUS_CHANGE_MAX); // let the input settle

    // is entered if there are two resets within a given time (timeslot-detection can issue this skip)
    if (skip_reset_detection)
    {
        skip_reset_detection = 0;
        if (!waitLoopsWhilePinIs(LOOPS_RESET_MIN - LOOPS_SLOT_MAX - LOOPS_READ_STD, false))
        {
            waitLoopsWhilePinIs(LOOPS_RESET_MAX, false); // showPresence() wants to start at high, so wait for it
            return true;
        }
    }

    if (!DIRECT_READ(pin_baseReg, pin_bitMask)) return false; // just leave if pin is Low, don't bother to wait, TODO: really needed?

    // wait for the bus to become low (master-controlled), since we are polling we don't know for how long it was zero
    if (!waitLoopsWhilePinIs(LOOPS_RESET_TIMEOUT, true))
    {
        //_error = Error::WAIT_RESET_TIMEOUT;
        return false;
    }

    uint32_t time_start = micros(); // TODO: can be done without using micros, just return left retries

    // wait for bus-release by master
    if (!waitLoopsWhilePinIs(LOOPS_RESET_MAX, false))
    {
        _error = Error::VERY_LONG_RESET;
        return false;
    }

    // If the master pulled low for to short this will trigger an error
    if ((time_start + ONEWIRE_TIME_RESET_MIN) > micros())
    {
        //_error = Error::VERY_SHORT_RESET; // TODO: activate again, like the error above, errorhandling is mature enough now
        return false;
    }

    overdrive_mode = false; // normal reset detected, so leave OD-Mode

    return true;
}


bool OneWireHub::showPresence(void)
{
    // Master will delay it's "Presence" check (bus-read)  after the reset
    waitLoopsWhilePinIs(LOOPS_PRESENCE_SAMPLE_MIN, true); // no pinCheck demanded, but this additional check can cut waitTime

    // pull the bus low and hold it some time
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);    // drive output low

    wait(ONEWIRE_TIME_PRESENCE_LOW_STD);

    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);     // allow it to float

    // When the master or other slaves release the bus within a given time everything is fine
    if (!waitLoopsWhilePinIs((LOOPS_PRESENCE_LOW_MAX - LOOPS_PRESENCE_LOW_STD), false))
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
            if (_error != Error::NO_ERROR)  return false;
            sendBit(false);
            if (_error != Error::NO_ERROR)  return false;
            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR)  return false;

            // switch to next junction
            trigger_pos = bit_recv ? idTree[trigger_pos].got_one : idTree[trigger_pos].got_zero;

            active_slave = idTree[trigger_pos].slave_selected;

            trigger_bit = (trigger_pos == 255) ? uint8_t(255) : idTree[trigger_pos].id_position;
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
                if (_error != Error::NO_ERROR)  return false;
                sendBit(false);
                if (_error != Error::NO_ERROR)  return false;
            }
            else
            {
                bit_send = 0;
                sendBit(false);
                if (_error != Error::NO_ERROR)  return false;
                sendBit(true);
                if (_error != Error::NO_ERROR)  return false;
            }

            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR)  return false;

            if (bit_send != bit_recv)  return false;
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

    if (skip_reset_detection)       return true; // stay in poll()-loop and trigger another datastream-detection
    if (_error != Error::NO_ERROR)  return false;

    switch (cmd)
    {
        case 0xF0: // Search rom
            search();
            return true; // always trigger a re-init after search

        case 0x69: // overdrive MATCH ROM
            overdrive_mode = true;
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
                    };
                };

                if (flag)
                {
                    slave_selected = slave_list[i];
                    break;
                };
            };

            if (!flag)
            {
                return false;
            };

            if (slave_selected != nullptr)
            {
                extend_timeslot_detection = 1;
#if USE_GPIO_DEBUG
                DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
                slave_selected->duty(this);
                DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#else
                slave_selected->duty(this);
#endif
            };
            return !(getError());

        case 0x3C: // overdrive SKIP ROM
            overdrive_mode = true;
        case 0xCC: // SKIP ROM
            // NOTE: If more than one slave is present on the bus,
            // and a read command is issued following the Skip ROM command,
            // data collision will occur on the bus as multiple slaves transmit simultaneously
            slave_selected = nullptr;

            for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
            {
                if (slave_list[i] != nullptr)
                {
                    slave_selected = slave_list[i];
                    break;
                };
            };

            if (slave_selected != nullptr)
            {
                extend_timeslot_detection = 1;
#if USE_GPIO_DEBUG
                DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
                slave_selected->duty(this);
                DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#else
                slave_selected->duty(this);
#endif
            };
            return !(getError());

        case 0x0F: // OLD READ ROM
            // only usable when there is ONE slave on the bus --> continue to current readRom

        case 0x33: // READ ROM
            // only usable when there is ONE slave on the bus
            if (slave_count == 1) {
                slave_selected = slave_list[getIndexOfNextSensorInList()];
                if (slave_selected != nullptr)
                {
                    slave_selected->sendID(this);
                };
            }
            return true;

        case 0xA5: // RESUME COMMAND
            // TODO: maybe add function to fully support the ds2432

        default: // Unknown command
            _error = Error::INCORRECT_ONEWIRE_CMD;
            _error_cmd = cmd;
    };
    return false;
};

// info: check for errors after calling and break/return if possible
bool OneWireHub::send(const uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_sent = 0;

    for (bytes_sent; bytes_sent < data_length; ++bytes_sent)
    {
        send(address[bytes_sent]);
        if (_error != Error::NO_ERROR)  break;
    };
    return (bytes_sent == data_length);
};

// info: check for errors after calling and break/return if possible, or check for return==false
bool OneWireHub::send(const uint8_t dataByte)
{
    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        sendBit((bitMask & dataByte) ? bool(1) : bool(0));
        if (_error != Error::NO_ERROR)
        {
            if (bitMask == 0x01)      _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
            return false;
        }
    };
    // TODO: was there an extend timeslot before? should be needed for loxone
    return true;
};

// info: check for errors after calling and break/return if possible, TODO: why return crc both ways, detect first bit break
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

// info: check for errors after calling and break/return if possible, or check for return==false
bool OneWireHub::sendBit(const bool value)
{
    // wait for a low to high transition followed by a high to low within the time-out
    if (!awaitTimeSlotAndWrite(!value))
    {
        _error = Error::WRITE_TIMESLOT_TIMEOUT;
        return false; // timeslot violation
    }

    if (value) waitLoopsWhilePinIs(LOOPS_READ_ONE_LOW_MAX, false); // no pinCheck demanded, but this additional check can cut waitTime
    else
    {
        // if we wait for release we could detect faulty writing slots --> pedantic Mode not needed for now
        wait(ONEWIRE_TIME_WRITE_ZERO_LOW_STD);
        DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
    }

    return true;
}


bool OneWireHub::recv(uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_received = 0;

    for (bytes_received; bytes_received < data_length; ++bytes_received)
    {
        address[bytes_received] = recv();
        if (_error != Error::NO_ERROR)
        {
            if (skip_reset_detection) _error = Error::NO_ERROR; // remove the pseudo error to leave recv() early,
            break;
        }
    }
    return (bytes_received == data_length);
}

uint8_t OneWireHub::recv(void)
{
    uint8_t value = 0;

    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (recvBit())  value |= bitMask;
        if (_error != Error::NO_ERROR)
        {
            if (skip_reset_detection) _error = Error::NO_ERROR; // remove the pseudo error to leave recv() early
            if (bitMask == 0x01)      _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
            break;
        }
    }
    // TODO: was there an extend timeslot before? should be needed for loxone
    return value;
}

// TODO: not happy with the interface - call by ref is slow here. maybe use a crc in class and expand with crc-reset and get?, TODO: detect first bit break
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
        if (skip_reset_detection)
        {
            // after running through awaitTimeSlotAndWrite() for the second time and still getting a low this branch raises a pseudo-error to leave the caller recv()
            _error = Error::FIRST_TIMESLOT_TIMEOUT;
        }
        else
        {
            _error = Error::READ_TIMESLOT_TIMEOUT;
        }
        return 0;
    }

    waitLoopsWhilePinIs(LOOPS_READ_STD, false); // no pinCheck demanded, but this additional check can cut waitTime
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask); // TODO: should not be needed

    return DIRECT_READ(pin_baseReg, pin_bitMask);
}

#define USE_DELAY 1

void OneWireHub::wait(const uint16_t timeout_us)
{
#if USE_DELAY
    delayMicroseconds(timeout_us);
#else
    uint32_t time_trigger = micros() + timeout_us;
    while (micros() < time_trigger)
    {
    }
#endif
}

#define USE_MICROS 1


#define NEW_WAIT 0 // TODO: NewWait does not work as expected
#if NEW_WAIT

// wait for a low to high transition followed by a high to low within the time-out
bool OneWireHub::awaitTimeSlotAndWrite(const bool writeZero)
{
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    //While bus is low, retry until HIGH

    if (!delayLoopsWhilePinIs(LOOPS_SLOT_MAX, false))
    {
        if (extend_timeslot_detection == 2)
        {
            // this branch can be taken after calling THIS functions the second time in recvBit()
            extend_timeslot_detection = 0;
            skip_reset_detection      = 1;
        }
        else
        {
            _error = Error::READ_TIMESLOT_TIMEOUT_LOW;
        };
        return false;
    }

    // extend the wait-time after reset and presence-detection
    if (extend_timeslot_detection == 1)
    {
        extend_timeslot_detection = 2; // prepare to detect missing timeslot or second reset
    }

    //Wait for bus to fall form 1 to 0
    if (delayLoopsWhilePinIs(LOOPS_RESET_TIMEOUT, true))
    {
            _error = Error::READ_TIMESLOT_TIMEOUT_HIGH;
            return false;
    };

    // if extend_timeslot_detection == 2 we could safe millis()

    if (writeZero)
    {
        // Low is allready set
        DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);
    };

    return true;
}

#else

#define TIMESLOT_WAIT_RETRY_COUNT  static_cast<uint16_t>(microsecondsToClockCycles(135)/8)   /// :11 is a specif value for 8bit-atmega, still to determine
bool OneWireHub::awaitTimeSlotAndWrite(const bool writeZero)
{
    noInterrupts();
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    //While bus is low, retry until HIGH
    uint16_t retries = TIMESLOT_WAIT_RETRY_COUNT; // TODO: redo this section
    while (!DIRECT_READ(pin_baseReg, pin_bitMask))
    {
        if (--retries == 0)
        {
            if (extend_timeslot_detection == 2)
            {
                // this branch can be taken after calling THIS functions the second time in recvBit()
                extend_timeslot_detection = 0;
                skip_reset_detection      = 1;
            }
            else
            {
                _error = Error::READ_TIMESLOT_TIMEOUT_LOW;
            };
            interrupts();
            return false;
        };
    };

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
    };

    //Wait for bus to fall form 1 to 0
    while (DIRECT_READ(pin_baseReg, pin_bitMask))
    {
        if (--retries == 0)
        {
            _error = Error::READ_TIMESLOT_TIMEOUT_HIGH;
            interrupts();
            return false;
        };
    };

    // if extend_timeslot_detection == 2 we could safe millis()

    if (writeZero)
    {
        // Low is allready set
        DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);
    };
    interrupts();
    return true;
};
#endif



timeOW_t OneWireHub::waitLoopsCalibrate(void)
{
    constexpr timeOW_t repetitions{5000}; // how many low_states will measured before assuming that there was a reset in it
    constexpr timeOW_t wait_retries{1000000 * microsecondsToClockCycles(1)}; // loops before cancelling a pin-change-wait

    timeOW_t time_max = 0;
    timeOW_t measure = 10;

    // measure the longest low-states on the bus with millis(), assume it is a OW-reset
    while (measure--)
    {
        uint32_t time_needed = 0;

        // try to catch a OW-reset each time
        while (time_needed < ONEWIRE_TIME_RESET_MIN)
        {
            waitLoopsWhilePinIs(wait_retries, true);
            const uint32_t time_start = micros();
            waitWhilePinIs(false);
            const uint32_t time_stop = micros();
            time_needed = time_stop - time_start;
        };

        if (time_needed > time_max) time_max = time_needed;
    };

    timeOW_t retries_max = 0;
    measure = 0;

    while (measure++ < repetitions)
    {
        waitLoopsWhilePinIs(wait_retries, true);
        const timeOW_t retries_needed = measureLoopsWhilePinIs(false);
        if (retries_needed>retries_max) retries_max = retries_needed;
    };

    // analyze
    factor_nspl = time_max * VALUE1k / retries_max;
    waitLoopsConfig();

    calibrate_loop_timing = false;

    //factor_nspl = factor_ipl * VALUE1k / microsecondsToClockCycles(1);
    //const timeOW_t factor_ipl = time_max * microsecondsToClockCycles(1) / retries_max;
    const timeOW_t factor_ipl = factor_nspl * microsecondsToClockCycles(1) / VALUE1k;
    return factor_ipl;
};

void OneWireHub::waitLoopsConfig(void)
{
    if (!factor_nspl) return;

    LOOPS_BUS_CHANGE_MAX        = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_BUS_CHANGE_MAX); // TODO: could be that we can reduce to uint16_t for atmega
    LOOPS_RESET_MIN             = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_RESET_MIN);
    LOOPS_RESET_MAX             = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_RESET_MAX);
    LOOPS_RESET_TIMEOUT         = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_RESET_TIMEOUT);
    LOOPS_PRESENCE_SAMPLE_MIN   = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_PRESENCE_SAMPLE_MIN);
    LOOPS_PRESENCE_LOW_STD      = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_PRESENCE_LOW_STD);
    LOOPS_PRESENCE_LOW_MAX      = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_PRESENCE_LOW_MAX);
    LOOPS_PRESENCE_HIGH_MAX     = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_PRESENCE_HIGH_MAX);
    LOOPS_SLOT_MAX              = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_SLOT_MAX);
    LOOPS_READ_ONE_LOW_MAX      = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_READ_ONE_LOW_MAX);
    LOOPS_READ_STD              = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_READ_STD);
    LOOPS_WRITE_ZERO_LOW_STD    = waitLoopsCalculate(VALUE1k * ONEWIRE_TIME_WRITE_ZERO_LOW_STD);

#if USE_GPIO_DEBUG
    // demonstrate an 1ms-Low-State on the debug pin (only if bus stays high during this time)
    const timeOW_t loops_1ms = waitLoopsCalculate(VALUE1k * VALUE1k);
    waitLoopsWhilePinIs(loops_1ms,false);
    DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
    waitLoopsWhilePinIs(loops_1ms,true);
    DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#endif
};


timeOW_t OneWireHub::waitLoopsCalculate(const timeOW_t time_ns)
{
    // precalc waitvalues, the OP can take up da 550 cylces
    timeOW_t retries = (time_ns / factor_nspl);
    //if (retries) retries--;
    return retries;
};

// returns false if pins stays in the wanted state all the time
bool OneWireHub::waitLoopsWhilePinIs(timeOW_t retries, const bool pin_value)
{
    if (retries == 0) return false;
    //noInterrupts();
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value) && (--retries));
    //interrupts();
    return (retries > 0);
};

void OneWireHub::waitWhilePinIs(const bool pin_value)
{
    while (DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value);
};

timeOW_t OneWireHub::measureLoopsWhilePinIs(const bool pin_value)
{
    timeOW_t retries = 1;
    //noInterrupts();
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value) && (++retries));
    //interrupts();
    return (retries);
}

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
};

bool OneWireHub::getError(void)
{
    return (_error != Error::NO_ERROR);
};

void OneWireHub::raiseSlaveError(const uint8_t cmd)
{
    _error = Error::INCORRECT_SLAVE_USAGE;
    _error_cmd = cmd;
};

Error OneWireHub::clearError(void) // and return it if needed
{
    Error _tmp = _error;
    _error = Error::NO_ERROR;
    return _tmp;
};
