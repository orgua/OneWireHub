// large parts are derived from https://github.com/MarkusLange/OneWireSlave/blob/master/OneWireSlave.cpp


#include "OneWireHub.h"
#include "pins_arduino.h"

extern "C" {
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
}

#define DIRECT_READ(base, mask)        (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)  ((*(base+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask) ((*(base+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)   ((*(base+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)  ((*(base+2)) |= (mask))

#define TIMESLOT_WAIT_RETRY_COUNT       (microsecondsToClockCycles(120) / 10L)
#define TIMESLOT_WAIT_READ_RETRY_COUNT  (microsecondsToClockCycles(135))

//--- CRC 16 --- // TODO: only used in ds2450 and ds2408 and ds2423
static uint16_t crc16;

void ow_crc16_reset(void)
{
    crc16 = 0;
}

void ow_crc16_update(uint8_t b)
{
    for (uint8_t j = 0; j < 8; ++j)
    {
        uint8_t mix = ((uint8_t) crc16 ^ b) & static_cast<uint8_t>(0x01);
        crc16 = crc16 >> 1;
        if (mix)
            crc16 = crc16 ^ static_cast<uint16_t>(0xA001);

        b = b >> 1;
    }
}

uint16_t ow_crc16_get(void)
{
    return crc16;
}

//=================== Hub ==========================================
OneWireHub::OneWireHub(uint8_t pin)
{
    pin_bitmask = digitalPinToBitMask(pin);
    slave_count = 0;
    errno = ONEWIRE_NO_ERROR;
    baseReg = portInputRegister(digitalPinToPort(pin));

    //bits[ONEWIREIDMAP_COUNT]; // TODO: init
    //idmap0[ONEWIREIDMAP_COUNT];
    //idmap1[ONEWIREIDMAP_COUNT];

    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
        elms[i] = nullptr;

    SelectElm = nullptr;
};

bool OneWireHub::waitForRequest(const bool ignore_errors) // TODO: maybe build a non blocking version of this? and more common would be inverse of ignore_errors (=blocking)
{
    errno = ONEWIRE_NO_ERROR;

    while (1)
    {
        //Once reset is done, it waits another 30 micros
        //Master wait is 65, so we have 35 more to send our presence now that reset is done
        if (!waitReset(0))
        {
            continue;
        }

        //Reset is complete, tell the master we are prsent
        // This will pull the line low for 125 micros (155 micros since the reset) and
        //  then wait another 275 plus whatever wait for the line to go high to a max of 480
        // This has been modified from original to wait for the line to go high to a max of 480.
        if (!presence())
        {
            continue;
        }

        // this additional check prevents an infinite loop when calling this FN without sensors attached
        if (!slave_count)
        {
            return TRUE;
        }

        //Now that the master should know we are here, we will get a command from the line
        //Because of our changes to the presence code, the line should be guranteed to be high
        if (recvAndProcessCmd())
        {
            return TRUE;
        }
        else if ((errno == ONEWIRE_NO_ERROR) || ignore_errors)
        {
            continue;
        }
        else
        {
            return FALSE;
        }
    }
}


// attach a sensor to the hub
uint8_t OneWireHub::attach(OneWireItem &sensor)
{
    if (slave_count >= ONEWIRESLAVE_COUNT) return 0; // hub is full

    // find position of next free storage-position
    uint8_t position = 255;
    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
    {
        // check for already attached sensors
        if (elms[i] == &sensor)
            return i;

        // store position of first empty space
        if ((position>ONEWIRESLAVE_COUNT) && (elms[i] == nullptr))
        {
            position = i;
            break;
        }
    }


    elms[position] = &sensor;
    slave_count++;
    calc_mask();
    return position;
};

bool    OneWireHub::detach(const OneWireItem &sensor)
{
    // find position of sensor
    uint8_t position = 255;
    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
    {
        if (elms[i] == &sensor)
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
    if (elms[slave_number] == nullptr)          return 0;
    if (!slave_count)                           return 0;
    if (slave_number >= ONEWIRESLAVE_COUNT)     return 0;

    elms[slave_number] = nullptr;
    slave_count--;
    calc_mask();
    return 1;
};

// TODO: this memory-monster can be reduced.
// just look through each bit of each ID and build a tree, so there are n=slavecount decision-points
// tradeoff: more online calculation, but @4Slave 16byte storage instead of 3*256 byte

uint8_t OneWireHub::get_first_element(const uint8_t mask)
{
    for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
    {
        if (mask & (1 << i))
        {
            return i;
        }
    }
}


void OneWireHub::build_tree(uint8_t position_IDBit, const uint8_t mask_slaves)
{
    if (!mask_slaves) return;

    while (position_IDBit < 64)
    {
        uint8_t mask_pos = 0;
        uint8_t mask_neg = 0;
        const uint8_t pos_byte = (position_IDBit >> 3);
        const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));

        // search through all active slaves
        for (uint8_t id = 0; id < ONEWIRESLAVE_COUNT; ++id)
        {
            const uint8_t mask_id = (static_cast<uint8_t>(1) << id);

            if (mask_slaves & mask_id)
            {
                // if slave is in mask differentiate the bitvalue
                if (elms[id]->ID[pos_byte] & mask_bit)
                    mask_pos |= mask_id;
                else
                    mask_neg |= mask_id;
            }
        }

        if (mask_neg && mask_pos)
        {
            // there was found a junction
            uint8_t active_element = 0;
            for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
            {
                if (idTree[i].bitposition == 255)
                {
                    active_element = i;
                    break;
                }
            };

            idTree[active_element].bitposition = position_IDBit;
            idTree[active_element].gotOne      = get_first_element(mask_pos);
            idTree[active_element].gotZero     = get_first_element(mask_neg);
            idTree[active_element].slave_selected = get_first_element(mask_slaves);
            position_IDBit++;
            build_tree(position_IDBit, mask_pos);
            build_tree(position_IDBit, mask_neg);
            return;
        };

        position_IDBit++;
    }
}

int OneWireHub::calc_mask(void)
{
    uint8_t mask_slaves = 0;

    for (uint8_t i = 0; i< ONEWIRESLAVE_COUNT; ++i)
    {
        if (elms[i] != nullptr) mask_slaves |= (1 << i);
        idTree[i].bitposition    = 255;
        idTree[i].slave_selected = 255;
    }

    build_tree(0, mask_slaves);

    // store root-element
    idTree[ONEWIRESLAVE_COUNT-1].slave_selected = get_first_element(mask_slaves);

    if (dbg_CALC)
    {
        Serial.println("Calculate idTree: ");
        for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
        {

            Serial.print("Slave: ");
            if (idTree[i].slave_selected < 10) Serial.print("  ");
            Serial.print(idTree[i].slave_selected);
            Serial.print(" bitPos: ");
            if (idTree[i].bitposition < 10) Serial.print(" ");
            if (idTree[i].bitposition < 100) Serial.print(" ");
            Serial.print(idTree[i].bitposition);
            Serial.print(" if0gt: ");
            Serial.print(idTree[i].gotZero);
            Serial.print(" if1gt: ");
            Serial.println(idTree[i].gotOne);
        }
    }
}

bool OneWireHub::waitReset(uint16_t timeout_ms)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint32_t time_stamp;

    errno = ONEWIRE_NO_ERROR;
    cli();
    DIRECT_MODE_INPUT(reg, mask);
    sei();

    //Wait for the line to fall
    if (timeout_ms != 0)
    {
        time_stamp = micros() + timeout_ms * 1000;
        while (DIRECT_READ(reg, mask))
        {
            if (micros() > time_stamp)
            {
                errno = ONEWIRE_WAIT_RESET_TIMEOUT;
                return FALSE;
            }
        }
    }
    else
    {
        //Will wait forever for the line to fall
        while (DIRECT_READ(reg, mask))
        { };
    }

    //Set to wait for rise up to 540 micros
    //Master code sets the line low for 500 micros
    //TODO The actual documented max is 640, not 540
    time_stamp = micros() + 540;

    //Wait for the rise on the line up to 540 micros
    while (DIRECT_READ(reg, mask) == 0)
    {
        if (micros() > time_stamp)
        {
            errno = ONEWIRE_VERY_LONG_RESET;
            return FALSE;
        }
    }

    //If the master pulled low for exactly 500, then this will be 40 wait time
    // Recommended for master is 480, which would be 60 here then
    // Max is 640, which makes this negative, but it returns above as a "ONEWIRE_VERY_LONG_RESET"
    // this gives an extra 10 to 30 micros befor calling the reset invalid
    if ((time_stamp - micros()) > 70)
    {
        errno = ONEWIRE_VERY_SHORT_RESET;
        return FALSE;
    }

    //Master will now delay for 65 to 70 recommended or max of 75 before it's "presence" check
    // and then read the pin value (checking for a presence on the line)
    // then wait another 490 (so, 500 + 64 + 490 = 1054 total without consideration of actual op time) on Arduino,
    // but recommended is 410 with total reset length of 480 + 70 + 410 (or 480x2=960)
    delayMicroseconds(30);
    //Master wait is 65, so we have 35 more to send our presence now that reset is done
    return TRUE;
}

bool OneWireHub::presence(const uint8_t delta_us)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;

    //Reset code already waited 30 prior to calling this
    // Master will not read until 70 recommended, but could read as early as 60
    // so we should be well enough ahead of that. Arduino waits 65
    errno = ONEWIRE_NO_ERROR;
    cli();
    DIRECT_WRITE_LOW(reg, mask);
    DIRECT_MODE_OUTPUT(reg, mask);    // drive output low
    sei();

    //Delaying for another 125 (orignal was 120) with the line set low is a total of at least 155 micros
    // total since reset high depends on commands done prior, is technically a little longer
    delayMicroseconds(125);
    cli();
    DIRECT_MODE_INPUT(reg, mask);     // allow it to float
    sei();

    //Default "delta" is 25, so this is 275 in that condition, totaling to 155+275=430 since the reset rise
    // docs call for a total of 480 possible from start of rise before reset timing is completed
    //This gives us 50 micros to play with, but being early is probably best for timing on read later
    //delayMicroseconds(300 - delta);
    delayMicroseconds(static_cast<uint16_t>(250) - delta_us);  // TODO: where does 250 come from?

    //Modified to wait a while (roughly 50 micros) for the line to go high
    // since the above wait is about 430 micros, this makes this 480 closer
    // to the 480 standard spec and the 490 used on the Arduino master code
    // anything longer then is most likely something going wrong.
    uint8_t retries = 25;
    //while (!DIRECT_READ(reg, mask));
    do
    {
        if (retries-- == 0)  return FALSE;
        delayMicroseconds(2);
    } while (!DIRECT_READ(reg, mask));


    if ( DIRECT_READ(reg, mask))
    {
        return TRUE;
    }
    else
    {
        errno = ONEWIRE_PRESENCE_LOW_ON_LINE;
        return FALSE;
    };
}

uint8_t OneWireHub::get_next_treejunction(const uint8_t slave, const uint8_t position_bit_min)
{
    for (uint8_t branch = 0; branch < (ONEWIRESLAVE_COUNT); ++branch)
    {
        if (idTree[branch].slave_selected == slave)
            if (idTree[branch].bitposition >= position_bit_min)
                return branch;
    }
    return (ONEWIRESLAVE_COUNT-1);
}

bool OneWireHub::search(void)
{
    uint8_t position_IDBit = 0;
    uint8_t active_slave = idTree[ONEWIRESLAVE_COUNT - 1].slave_selected;
    uint8_t trigger_pos  = get_next_treejunction(active_slave,position_IDBit);
    uint8_t trigger_bit  = idTree[trigger_pos].bitposition;

    while (position_IDBit < 64)
    {
        // if junction is reached, act different
        if (position_IDBit == trigger_bit)
        {
            sendBit(FALSE);
            sendBit(FALSE);
            uint8_t bit_recv = recvBit();
            if (errno != ONEWIRE_NO_ERROR)
                return FALSE;
            if (bit_recv)   active_slave = idTree[trigger_pos].gotOne;
            else            active_slave = idTree[trigger_pos].gotZero;
            // find next junction if needed
            trigger_pos  = get_next_treejunction(active_slave,position_IDBit+1);
            trigger_bit  = idTree[trigger_pos].bitposition;
        }
        else
        {
            const uint8_t pos_byte = (position_IDBit >> 3);
            const uint8_t mask_bit = (static_cast<uint8_t>(1) << (position_IDBit & (7)));
            uint8_t bit_send, bit_recv;

            if (elms[active_slave]->ID[pos_byte] & mask_bit)
            {
                bit_send = 1;
                sendBit(TRUE);
                sendBit(FALSE);
            }
            else
            {
                bit_send = 0;
                sendBit(FALSE);
                sendBit(TRUE);
            }

            bit_recv = recvBit();
            if (errno != ONEWIRE_NO_ERROR)
                return FALSE;

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

    SelectElm = elms[active_slave];
    return true;
}

bool OneWireHub::recvAndProcessCmd(void)
{
    uint8_t addr[8];
    bool flag;

    while (1)
    {
        uint8_t cmd = recv();

        switch (cmd)
        {
            // Search rom
            case 0xF0:
                cmd = static_cast<uint8_t>(search()); // missuse cmd here, but
                delayMicroseconds(5900); // TODO: was commented out at MLange
                if (cmd)  return TRUE; // TODO: hotfix for DS2401 / infinite loop, but with the delay active there can only be one 2401
                else      return FALSE;

                // MATCH ROM - Choose/Select ROM
            case 0x55:
                recvData(addr, 8);
                if (errno != ONEWIRE_NO_ERROR)
                    return FALSE;

                flag = FALSE;
                SelectElm = 0;

                for (uint8_t i = 0; i < ONEWIRESLAVE_COUNT; ++i)
                {
                    if (elms[i] == nullptr) continue;

                    flag = TRUE;
                    for (uint8_t j = 0; j < 8; ++j)
                    {
                        if (elms[i]->ID[j] != addr[j])
                        {
                            flag = FALSE;
                            break;
                        }
                    }

                    if (flag)
                    {
                        SelectElm = elms[i];

                        if (dbg_MATCHROM)
                        {
                            Serial.print("MATCH ROM=");
                            Serial.println(i);
                        }

                        break;
                    }
                }

                if (flag == FALSE)          return FALSE;
                if (SelectElm != nullptr)   SelectElm->duty(this);
                return TRUE;

                // SKIP ROM
            case 0xCC:
                SelectElm = nullptr;
                return TRUE;

            default: // Unknow command
                if (dbg_HINT)
                {
                    Serial.print("U:");
                    Serial.println(cmd, HEX);
                }
                return FALSE;
        }
    }
}

uint8_t OneWireHub::sendData(const uint8_t buf[], const uint8_t len)
{
    uint8_t bytes_sended = 0;

    for (uint8_t i = 0; i < len; ++i)
    {
        send(buf[i]);
        if (errno != ONEWIRE_NO_ERROR)
            break;
        bytes_sended++;
    }
    return bytes_sended;
}

uint8_t OneWireHub::recvData(uint8_t buf[], const uint8_t len)
{
    uint8_t bytes_received = 0;

    for (int i = 0; i < len; ++i)
    {
        buf[i] = recv();
        if (errno != ONEWIRE_NO_ERROR)
            break;
        bytes_received++;
    }
    return bytes_received;
}

void OneWireHub::send(const uint8_t v)
{
    errno = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask && (errno == ONEWIRE_NO_ERROR); bitmask <<= 1)
        sendBit((bitmask & v) ? 1 : 0);
}

uint8_t OneWireHub::recv(void)
{
    uint8_t r = 0;

    errno = ONEWIRE_NO_ERROR;
    for (uint8_t bitmask = 0x01; bitmask && (errno == ONEWIRE_NO_ERROR); bitmask <<= 1)
        if (recvBit())
            r |= bitmask;
    return r;
}

void OneWireHub::sendBit(const uint8_t v)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;

    cli();
    DIRECT_MODE_INPUT(reg, mask);
    //waitTimeSlot waits for a low to high transition followed by a high to low within the time-out
    uint8_t wt = waitTimeSlot();
    if (wt != 1)
    { //1 is success, others are failure
        if (wt == 10)   errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW;
        else            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH;
        sei();
        return;
    }
    if (v & 1)  delayMicroseconds(30);
    else
    {
        cli();
        DIRECT_WRITE_LOW(reg, mask);
        DIRECT_MODE_OUTPUT(reg, mask);
        delayMicroseconds(30);
        DIRECT_WRITE_HIGH(reg, mask);
    }
    sei();
    return;
}

uint8_t OneWireHub::recvBit(void)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint8_t r;

    cli();
    DIRECT_MODE_INPUT(reg, mask);
    //waitTimeSlotRead is a customized version of the original which was also
    // used by the "write" side of things.
    uint8_t wt = waitTimeSlotRead();
    if (wt != 1)
    { //1 is success, others are failure
        if (wt == 10)
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_LOW;
        } else
        {
            errno = ONEWIRE_READ_TIMESLOT_TIMEOUT_HIGH;
        }
        sei();
        return 0;
    }
    delayMicroseconds(30);
    //TODO Consider reading earlier: delayMicroseconds(15);
    r = DIRECT_READ(reg, mask);
    sei();
    return r;
}

uint8_t OneWireHub::waitTimeSlot(void)
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint16_t retries;

    //Wait for a 0 to rise to 1 on the line for timeout duration
    //If the line is already high, this is basically skipped
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    //While line is low, retry
    while (!DIRECT_READ(reg, mask))
        if (--retries == 0)
            return 10;

    //Wait for a fall form 1 to 0 on the line for timeout duration
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    while (DIRECT_READ(reg, mask));
    if (--retries == 0)
        return 20;

    return 1;
}

//This is a copy of what was orig just "waitTimeSlot"
// it is customized for the reading side of things
uint8_t OneWireHub::waitTimeSlotRead()
{
    uint8_t mask = pin_bitmask;
    volatile uint8_t *reg asm("r30") = baseReg;
    uint16_t retries;

    //Wait for a 0 to rise to 1 on the line for timeout duration
    //If the line is already high, this is basically skipped
    retries = TIMESLOT_WAIT_RETRY_COUNT;
    //While line is low, retry
    while (!DIRECT_READ(reg, mask))
        if (--retries == 0)
            return 10;

    //TODO Seems to me that the above loop should drop out immediately because
    // The line is already high as our wait after presence is relatively short
    // So now it just waits a short period for the write of a bit to start
    // Unfortunately per "recommended" this is 55 micros to 130 micros more
    // more than what we may have already waited.

    //Wait for a fall form 1 to 0 on the line for timeout duration
    retries = TIMESLOT_WAIT_READ_RETRY_COUNT;
    while (DIRECT_READ(reg, mask));
    if (--retries == 0)
        return 20;

    return 1;
}

//==================== Item =========================================
OneWireItem::OneWireItem(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) // TODO: could give all sensors a const on every input
{
    ID[0] = ID1;
    ID[1] = ID2;
    ID[2] = ID3;
    ID[3] = ID4;
    ID[4] = ID5;
    ID[5] = ID6;
    ID[6] = ID7;
    ID[7] = crc8(ID, 7);
};


// The 1-Wire CRC scheme is described in Maxim Application Note 27:
// "Understanding and Using Cyclic Redundancy Checks with Maxim iButton Products"
// fast but needs more storage:
//  https://github.com/PaulStoffregen/OneWire/blob/master/OneWire.cpp --> calc with table (EOF)

// Compute a Dallas Semiconductor 8 bit CRC directly.
// slow, but small
uint8_t OneWireItem::crc8(const uint8_t addr[], const uint8_t len)
{
    uint8_t crc = 0;

    for (uint8_t i = 0; i < len; ++i)
    {
        uint8_t inbyte = addr[i];
        for (uint8_t j = 8; j; --j)
        {
            uint8_t mix = (crc ^ inbyte) & static_cast<uint8_t>(0x01);
            crc >>= 1;
            if (mix) crc ^= 0x8C;
            inbyte >>= 1;
        }
    }
    return crc;
}

uint16_t OneWireItem::crc16(const uint8_t addr[], const uint8_t len)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    uint16_t crc = 0xFFFF; // initvalue

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = addr[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
};
