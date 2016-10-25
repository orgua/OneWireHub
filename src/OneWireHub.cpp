// large parts are derived from https://github.com/MarkusLange/OneWireSlave/blob/master/OneWireSlave.cpp

#include "OneWireHub.h"
#include "OneWireItem.h"

OneWireHub::OneWireHub(const uint8_t pin)
{
    _error = Error::NO_ERROR;

    extend_timeslot_detection = 0;

    slave_count = 0;
    slave_selected = nullptr;

#if OVERDRIVE_ENABLE
    od_mode = false;
#endif

    for (uint8_t i = 0; i < ONEWIRESLAVE_LIMIT; ++i)
    {
        slave_list[i] = nullptr;
    }

    // prepare pin
    pin_bitMask = PIN_TO_BITMASK(pin);
    pin_baseReg = PIN_TO_BASEREG(pin);
    pinMode(pin, INPUT); // first port-access should by done by this FN, does more than DIRECT_MODE_....
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);

    // prepare debug:
#if USE_GPIO_DEBUG
    debug_bitMask = PIN_TO_BITMASK(GPIO_DEBUG_PIN);
    debug_baseReg = PIN_TO_BASEREG(GPIO_DEBUG_PIN);
    pinMode(GPIO_DEBUG_PIN, OUTPUT);
    DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#endif

    static_assert(VALUE_IPL, "Your architecture has not been calibrated yet, please run examples/debug/calibrate_by_bus_timing and report instructions per loop (IPL) to https://github.com/orgua/OneWireHub");
    static_assert(timeUsToLoops(ONEWIRE_TIME_VALUE_MIN)>1,"YOUR ARCHITECTURE IS TO SLOW, THIS MAY RESULT IN TIMING-PROBLEMS");
    static_assert((ONEWIRE_TIME_VALUE_MAX / VALUE_IPL ) < (TIMEOW_MAX / microsecondsToClockCycles(1)),"YOUR ARCHITECTURE IS TO FAST, OR TIMING VALUES SET TO HIGH --> OVERFLOW DETECTED");
};


// attach a sensor to the hub
uint8_t OneWireHub::attach(OneWireItem &sensor)
{
    if (slave_count >= ONEWIRESLAVE_LIMIT) return 0; // hub is full

    // demonstrate an 1ms-Low-State on the debug pin (only if bus stays high during this time)
    // done here because this FN is always called before hub is used
#if USE_GPIO_DEBUG
        static bool calibrate_loop_timing = true;
        if (calibrate_loop_timing)
        {
            calibrate_loop_timing = false;
            waitLoops1ms();
        }
#endif

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
uint8_t OneWireHub::getNrOfFirstBitSet(const mask_t mask) const
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
uint8_t OneWireHub::getIndexOfNextSensorInList(const uint8_t index_start) const
{
    for (uint8_t i = index_start; i < ONEWIRE_TREE_SIZE; ++i)
    {
        if (slave_list[i] != nullptr)  return i;
    }
    return 0;
};

// gone through the address, store this result
uint8_t OneWireHub::getNrOfFirstFreeIDTreeElement(void) const
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

        // searchIDTree through all active slaves
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


bool OneWireHub::poll(void)
{
    _error = Error::NO_ERROR;

    while (1)
    {
        // this additional check prevents an infinite loop when calling this FN without sensors attached
        if (slave_count == 0)       return true;

        //Once reset is done, go to next step
        if (checkReset())           return false;

        // Reset is complete, tell the master we are present
        if (showPresence())         return false;

        //Now that the master should know we are here, we will get a command from the master
        if (recvAndProcessCmd())    return false;

        // on total success we want to start again, because the next reset could only be ~125 us away
    }
}


bool OneWireHub::checkReset(void) // there is a specific high-time needed before a reset may occur -->  >120us
{
    static_assert(LOOPS_RESET_MIN[0] > (LOOPS_SLOT_MAX[0] - LOOPS_READ_MIN[0]), "Timings are wrong");
    static_assert(LOOPS_RESET_MAX[0] > LOOPS_RESET_MIN[0], "Timings are wrong");
#if OVERDRIVE_ENABLE
    static_assert(LOOPS_RESET_MIN[1] > (LOOPS_SLOT_MAX[1] - LOOPS_READ_MIN[1]), "Timings are wrong");
    static_assert(LOOPS_RESET_MAX[0] > LOOPS_RESET_MIN[1], "Timings are wrong");
#endif

    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    // is entered if there are two resets within a given time (timeslot-detection can issue this skip)
    if (skip_reset_detection)
    {
        skip_reset_detection = 0;
        if (!waitLoopsWhilePinIs(LOOPS_RESET_MIN[od_mode] - LOOPS_SLOT_MAX[od_mode] - LOOPS_READ_MIN[od_mode], false))
        {
#if OVERDRIVE_ENABLE
            const timeOW_t loops_remaining = waitLoopsWhilePinIs(LOOPS_RESET_MAX[0], false); // showPresence() wants to start at high, so wait for it
            if (od_mode && ((LOOPS_RESET_MAX[0] - LOOPS_RESET_MIN[od_mode]) > loops_remaining))
            {
                od_mode = false; // normal reset detected, so leave OD-Mode
            };
#else
            waitLoopsWhilePinIs(LOOPS_RESET_MAX[0], false); // showPresence() wants to start at high, so wait for it
#endif
            return false;
        }
    }

    if (!DIRECT_READ(pin_baseReg, pin_bitMask)) return true; // just leave if pin is Low, don't bother to wait, TODO: really needed?

    // wait for the bus to become low (master-controlled), since we are polling we don't know for how long it was zero
    if (!waitLoopsWhilePinIs(LOOPS_RESET_TIMEOUT, true))
    {
        //_error = Error::WAIT_RESET_TIMEOUT;
        return true;
    }

    const timeOW_t loops_remaining = waitLoopsWhilePinIs(LOOPS_RESET_MAX[0], false);

    // wait for bus-release by master
    if (loops_remaining == 0)
    {
        _error = Error::VERY_LONG_RESET;
        return true;
    }

#if OVERDRIVE_ENABLE
    if (od_mode && ((LOOPS_RESET_MAX[0] - LOOPS_RESET_MIN[0]) > loops_remaining))
    {
        od_mode = false; // normal reset detected, so leave OD-Mode
    };
#endif

    // If the master pulled low for to short this will trigger an error
    if ((LOOPS_RESET_MAX[0] - LOOPS_RESET_MIN[od_mode]) < loops_remaining)
    {
        //_error = Error::VERY_SHORT_RESET; // could be activated again, like the error above, errorhandling is mature enough now
        return true;
    }

    return false;
}


bool OneWireHub::showPresence(void)
{
    static_assert(LOOPS_PRESENCE_MAX[0] > LOOPS_PRESENCE_MIN[0], "Timings are wrong");
#if OVERDRIVE_ENABLE
    static_assert(LOOPS_PRESENCE_MAX[1] > LOOPS_PRESENCE_MIN[1], "Timings are wrong");
#endif

    // Master will delay it's "Presence" check (bus-read)  after the reset
    waitLoopsWhilePinIs(LOOPS_PRESENCE_TIMEOUT[od_mode], true); // no pinCheck demanded, but this additional check can cut waitTime

    #if USE_GPIO_DEBUG
    DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
#endif

    // pull the bus low and hold it some time
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);    // drive output low

    wait(LOOPS_PRESENCE_MIN[od_mode]); // stays till the end, because it drives the bus low itself

    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);     // allow it to float

#if USE_GPIO_DEBUG
    DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#endif

    // When the master or other slaves release the bus within a given time everything is fine
    if (!waitLoopsWhilePinIs((LOOPS_PRESENCE_MAX[od_mode] - LOOPS_PRESENCE_MIN[od_mode]), false))
    {
        _error = Error::PRESENCE_LOW_ON_LINE;
        return true;
    }

    extend_timeslot_detection = 1; // DS9490R takes 7-9 ms after presence-detection to start with timeslots
    return false;
}

void OneWireHub::extendTimeslot(void)
{
    extend_timeslot_detection = 1;
}


void OneWireHub::searchIDTree(void)
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
            if (sendBit(false)) return;
            if (sendBit(false)) return;

            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR)  return;

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
                bit_send = true;
                if (sendBit(true))  return;
                if (sendBit(false)) return;
            }
            else
            {
                bit_send = false;
                if (sendBit(false)) return;
                if (sendBit(true))  return;
            }

            const bool bit_recv = recvBit();
            if (_error != Error::NO_ERROR)  return;

            if (bit_send != bit_recv)  return;
        }
        position_IDBit++;
    }

    slave_selected = slave_list[active_slave];
};

bool OneWireHub::recvAndProcessCmd(void)
{
    uint8_t address[8];
    bool    flag = false;

    uint8_t cmd = recv();

    if (skip_reset_detection)       return false; // stay in poll()-loop and trigger another datastream-detection
    if (_error != Error::NO_ERROR)  return true;

    switch (cmd)
    {
        case 0xF0: // Search rom
            slave_selected = nullptr;
            searchIDTree();
            return false; // always trigger a re-init after searchIDTree

        case 0x69: // overdrive MATCH ROM
#if OVERDRIVE_ENABLE
            od_mode = true;
#endif
        case 0x55: // MATCH ROM - Choose/Select ROM
            slave_selected = nullptr;

            if (recv(address, 8))  return true;

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
                return true;
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
            return (getError());

        case 0x3C: // overdrive SKIP ROM
#if OVERDRIVE_ENABLE
            od_mode = true;
#endif
        case 0xCC: // SKIP ROM
            // NOTE: If more than one slave is present on the bus,
            // and a read command is issued following the Skip ROM command,
            // data collision will occur on the bus as multiple slaves transmit simultaneously
            if ((slave_selected == nullptr) && (slave_count == 1))
            {
                slave_selected = slave_list[getIndexOfNextSensorInList()];
            }
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
            return (getError());

        case 0x0F: // OLD READ ROM
            // only usable when there is ONE slave on the bus --> continue to current readRom

        case 0x33: // READ ROM
            // only usable when there is ONE slave on the bus
            if ((slave_selected == nullptr) && (slave_count == 1))
            {
                slave_selected = slave_list[getIndexOfNextSensorInList()];
            }
            if (slave_selected != nullptr)
            {
                slave_selected->sendID(this);
            };
            return false;

        case 0xEC:
            // TODO: Alarm searchIDTree command, respond if flag is set
            // is like searchIDTree-rom, but only slaves with triggered alarm will appear

        case 0xA5: // RESUME COMMAND
            if (slave_selected == nullptr) return true;
#if USE_GPIO_DEBUG
            DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
            slave_selected->duty(this);
            DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
#else
            slave_selected->duty(this);
#endif

            return (getError());
        default: // Unknown command
            _error = Error::INCORRECT_ONEWIRE_CMD;
            _error_cmd = cmd;
    };
    return true;
};

// info: check for errors after calling and break/return if possible, returns true if error is detected
bool OneWireHub::send(const uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_sent = 0;

    for ( ; bytes_sent < data_length; ++bytes_sent)
    {
        if (send(address[bytes_sent])) break;
    };
    return (bytes_sent != data_length);
};

// info: check for errors after calling and break/return if possible, returns true if error is detected
bool OneWireHub::send(const uint8_t dataByte)
{
    for (uint8_t bitMask = 0x01; bitMask; bitMask <<= 1)
    {
        if (sendBit((bitMask & dataByte) != 0))
        {
            if (bitMask == 0x01)      _error = Error::FIRST_BIT_OF_BYTE_TIMEOUT;
            return true;
        }
    };
    // TODO: was there an extend timeslot before? should be needed for loxone
    return false;
};

// info: check for errors after calling and break/return if possible, TODO: why return crc both ways, detect first bit break
uint16_t OneWireHub::sendAndCRC16(uint8_t dataByte, uint16_t crc16)
{
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        if (sendBit((0x01 & dataByte) != 0)) break; // CRC is not valid if sending breaks

        uint8_t mix = ((uint8_t) crc16 ^ dataByte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        dataByte >>= 1;
    };
    return crc16;
}

// info: check for errors after calling and break/return if possible, returns true if error is detected
bool OneWireHub::sendBit(const bool value)
{
    // wait for a low to high transition followed by a high to low within the time-out
    if (awaitTimeSlotAndWrite(!value))
    {
        _error = Error::WRITE_TIMESLOT_TIMEOUT;
        return true; // timeslot violation
    }

    if (value) waitLoopsWhilePinIs(LOOPS_READ_MAX[od_mode], false); // no pinCheck demanded, but this additional check can cut waitTime
    else
    {
        // if we wait for release we could detect faulty writing slots --> pedantic Mode not needed for now
        waitLoopsWhilePinIs(LOOPS_WRITE_ZERO[od_mode],false);
        DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
    }

    return false;
}


bool OneWireHub::recv(uint8_t address[], const uint8_t data_length)
{
    uint8_t bytes_received = 0;

    for ( ; bytes_received < data_length; ++bytes_received)
    {
        address[bytes_received] = recv();
        if (_error != Error::NO_ERROR)
        {
            if (skip_reset_detection) _error = Error::NO_ERROR; // remove the pseudo error to leave recv() early,
            break;
        }
    }
    return (bytes_received != data_length);
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
    if (awaitTimeSlotAndWrite())
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

    waitLoopsWhilePinIs(LOOPS_READ_MIN[od_mode], false); // no pinCheck demanded, but this additional check can cut waitTime
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask); // TODO: should not be needed

    return DIRECT_READ(pin_baseReg, pin_bitMask);
}

void OneWireHub::wait(const uint16_t timeout_us) const
{
    timeOW_t loops = timeUsToLoops(timeout_us);
    bool state = false;
    while (loops)
    {
        loops = waitLoopsWhilePinIs(loops,state);
        state = !state;
    };
};

void OneWireHub::wait(const timeOW_t loops_wait) const
{
    timeOW_t loops = loops_wait;
    bool state = false;
    while (loops)
    {
        loops = waitLoopsWhilePinIs(loops,state);
        state = !state;
    };
};

// returns true if error is detected
bool OneWireHub::awaitTimeSlotAndWrite(const bool writeZero)
{
    noInterrupts();
    DIRECT_WRITE_LOW(pin_baseReg, pin_bitMask);
    DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);

    //While bus is low, retry until HIGH, if waitLoopsWhilePinIs() is to slow switch to old code
    timeOW_t retries = LOOPS_SLOT_MAX[od_mode];
    while (!(DIRECT_READ(pin_baseReg, pin_bitMask)) && (--retries));
    if (!retries)
    {
        if (extend_timeslot_detection == 2)
        {
            // this branch can be taken after calling THIS functions the second time in recvBit()
            extend_timeslot_detection = 0;
            skip_reset_detection = 1;
        } else
        {
            _error = Error::READ_TIMESLOT_TIMEOUT_LOW;
        };
        interrupts();
        return true;
    };


    // extend the wait-time after reset and presence-detection
    if (extend_timeslot_detection == 1)
    {
        extend_timeslot_detection = 2; // prepare to detect missing timeslot or second reset
    };


    //Wait for bus to fall form 1 to 0, if waitLoopsWhilePinIs() is to slow switch to old code
    retries = LOOPS_MSG_HIGH_TIMEOUT;
    while (DIRECT_READ(pin_baseReg, pin_bitMask))
    {
        if (--retries == 0)
        {
            _error = Error::READ_TIMESLOT_TIMEOUT_HIGH;
            interrupts();
            return true;
        };
    };

    // if extend_timeslot_detection == 2 we could safe millis()

    if (writeZero)
    {
        // Low is allready set
        DIRECT_MODE_OUTPUT(pin_baseReg, pin_bitMask);
    };
    interrupts();
    return false;
};


// returns false if pins stays in the wanted state all the time
timeOW_t OneWireHub::waitLoopsWhilePinIs(volatile timeOW_t retries, const bool pin_value) const
{
    if (retries == 0) return 0;
    while ((DIRECT_READ(pin_baseReg, pin_bitMask) == pin_value) && (--retries));
    return retries;
};

void OneWireHub::waitLoops1ms(void)
{
#if USE_GPIO_DEBUG
    constexpr timeOW_t loops_1ms = timeUsToLoops(uint16_t(VALUE1k));
    timeOW_t loops_left = 1;
    while (loops_left)
    {
        waitLoopsWhilePinIs(loops_1ms, false);
        DIRECT_MODE_INPUT(pin_baseReg, pin_bitMask);
        DIRECT_WRITE_HIGH(debug_baseReg, debug_bitMask);
        loops_left = waitLoopsWhilePinIs(loops_1ms, true);
        DIRECT_WRITE_LOW(debug_baseReg, debug_bitMask);
    };
#endif
};

// this calibration calibrates timing with the longest low-state on the OW-Bus.
// first it measures some resets with the millis()-fn to get real timing.
// after that it measures with a waitLoops()-FN to determine the instructions-per-loop-value for the used architecture
timeOW_t OneWireHub::waitLoopsCalibrate(void)
{
    constexpr timeOW_t wait_loops{1000000 * microsecondsToClockCycles(1)}; // loops before cancelling a pin-change-wait, 1s

    timeOW_t time_for_reset = 0;
    timeOW_t repetitions = 10;

    DIRECT_MODE_INPUT(pin_baseReg,pin_bitMask);

    // repetitions the longest low-states on the bus with millis(), assume it is a OW-reset
    while (repetitions--)
    {
        uint32_t time_needed = 0;

        // try to catch a OW-reset each time
        while (time_needed < ONEWIRE_TIME_RESET_MIN)
        {
            if (!waitLoopsWhilePinIs(wait_loops, true)) continue;
            const uint32_t time_start = micros();
            waitLoopsWhilePinIs(TIMEOW_MAX, false);
            const uint32_t time_stop = micros();
            time_needed = time_stop - time_start;
        };

        if (time_needed > time_for_reset) time_for_reset = time_needed;
    };

    timeOW_t loops_for_reset = 0;
    repetitions = 0;

    noInterrupts();
    while (repetitions++ < REPETITIONS)
    {
        if (!waitLoopsWhilePinIs(wait_loops, true)) continue;
        const timeOW_t loops_left = waitLoopsWhilePinIs(TIMEOW_MAX, false);
        const timeOW_t loops_needed = TIMEOW_MAX - loops_left;
        if (loops_needed>loops_for_reset) loops_for_reset = loops_needed;
    };
    interrupts();

    waitLoops1ms();

    const timeOW_t value_ipl = ( time_for_reset * microsecondsToClockCycles(1) ) / loops_for_reset;

    return value_ipl;
};


void OneWireHub::waitLoopsDebug(void) const
{
    Serial.println("DEBUG TIMINGS for the HUB (measured in loops):");
    Serial.println("(be sure to update VALUE_IPL in src/OneWireHub_config.h first!)");
    Serial.print("value : \t");
    Serial.print(VALUE_IPL * VALUE1k / microsecondsToClockCycles(1));
    Serial.println(" nanoseconds per loop");
    Serial.print("reset min : \t");
    Serial.println(LOOPS_RESET_MIN[od_mode]);
    Serial.print("reset max : \t");
    Serial.println(LOOPS_RESET_MAX[od_mode]);
    Serial.print("reset tout : \t");
    Serial.println(LOOPS_RESET_TIMEOUT);
    Serial.print("presence min : \t");
    Serial.println(LOOPS_PRESENCE_TIMEOUT[od_mode]);
    Serial.print("presence low : \t");
    Serial.println(LOOPS_PRESENCE_MIN[od_mode]);
    Serial.print("pres low max : \t");
    Serial.println(LOOPS_PRESENCE_MAX[od_mode]);
    Serial.print("msg hi timeout : \t");
    Serial.println(LOOPS_MSG_HIGH_TIMEOUT);
    Serial.print("slot max : \t");
    Serial.println(LOOPS_SLOT_MAX[od_mode]);
    Serial.print("read1low : \t");
    Serial.println(LOOPS_READ_MAX[od_mode]);
    Serial.print("read std : \t");
    Serial.println(LOOPS_READ_MIN[od_mode]);
    Serial.print("write zero : \t");
    Serial.println(LOOPS_WRITE_ZERO[od_mode]);
    Serial.flush();
};

void OneWireHub::printError(void) const
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
     else if (_error == Error::FIRST_BIT_OF_BYTE_TIMEOUT)  Serial.print("first bit of byte timeout");

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

bool OneWireHub::getError(void) const
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
