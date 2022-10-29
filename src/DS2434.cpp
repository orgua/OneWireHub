#include "DS2434.h"

DS2434::DS2434()
{
    clearMemory();
    clearScratchpad();
}

// As this device is not multidrop, it needs to handle ALL commands from the master
void DS2434::duty(OneWireHub * const hub)
{
    uint8_t start_byte, cmd, data;
    uint32_t time_now;
    if (hub->recv(&cmd))  return;

    switch (cmd)
    {

    case 0x11:      // Read Scratchpad
        if (hub->recv(&start_byte))  return;
        if (start_byte >= PAGE4_ADDR) return;
        for (uint8_t nByte = start_byte; nByte < PAGE4_ADDR; ++nByte)
        {
            // read through scratchpad until comm-error is raised
            if (hub->send(&scratchpad[nByte], 1)) return;
        }
        // TODO: is endless reading needed? probably not
        break;

    case 0x17:      // Write Scratchpad
        if (hub->recv(&start_byte))  return;
        if (start_byte >= PAGE4_ADDR) return; // when out of limits
        for (uint8_t nByte = start_byte; nByte < PAGE4_ADDR; ++nByte)
        {
            if (hub->recv(&data, 1)) return;
            scratchpad[nByte] = data;
        }
        break;

    case 0x22:      // copy scratchpad SP1 to NV1
        if (memory[0x62] & 0b100u) return; // check LOCK-Status
        writeMemory(&scratchpad[PAGE1_ADDR], PAGE_SIZE, PAGE1_ADDR);
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0x25:      // copy scratchpad SP2 to NV2
        writeMemory(&scratchpad[PAGE2_ADDR], PAGE_SIZE, PAGE2_ADDR);
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0x28:      // copy scratchpad SP3 to SRAM
        writeMemory(&scratchpad[PAGE3_ADDR], PAGE_SIZE, PAGE3_ADDR);
        break;

    case 0x71:      // Recall Memory, NV1 to SP1
        readMemory(&scratchpad[PAGE1_ADDR], PAGE_SIZE, PAGE1_ADDR);
        break;

    case 0x77:      // Recall Memory, NV2 to SP2
        readMemory(&scratchpad[PAGE2_ADDR], PAGE_SIZE, PAGE2_ADDR);
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0x7A:      // Recall Memory, SRAM to SP3
        readMemory(&scratchpad[PAGE3_ADDR], PAGE_SIZE, PAGE3_ADDR);
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0x43:      // lock NV1
        lockNV1();
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;
    case 0x44:      // unlock NV1
        unlockNV1();
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0xD2:      // trigger temperature-reading
        request_temp = true;
        timer_temp = millis() + DURATION_TEMP_ms;
        break;

    case 0xB2:      // read Page 4 and 5
        if (hub->recv(&start_byte))  return;
        if (start_byte < PAGE4_ADDR) return; // when out of limits
        if (start_byte >= PAGE6_ADDR) return;

        // update status byte, TODO: done here because laptop waits -> check duration
        time_now = millis();
        if (time_now >= timer_nvwr)     memory[0x62] &= ~0b10u; // erase busy-flag
        else                            memory[0x62] |= 0b10u; // set busy-flag
        if (time_now >= timer_temp)     memory[0x62] &= ~0b1u; // erase busy-flag
        else                            memory[0x62] |= 0b1u; // set busy-flag

        for (uint8_t nByte = start_byte; nByte < PAGE6_ADDR; ++nByte)
        {
            if (hub->send(&memory[nByte], 1)) return;
        }
        break;

    case 0xB5:      // increment Cycle
        if (++memory[0x83] == 0u)
        {
            // after overflow of LSB
            memory[0x82]++;
        }
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0xB8:      // reset Cycle
        memory[0x82] = 0u;
        memory[0x83] = 0u;
        // NOTE: OP occupies real NV for ~ 10 ms (NVB-Bit)
        timer_nvwr = millis() + DURATION_NVWR_ms;
        break;

    case 0x8E:      // secret command just to avoid triggering an error
        break;
    case 0x84:      // second secret command
        if (hub->recv(&data))  return;
        break;

    default:
        hub->raiseSlaveError(cmd);
    }
}

void DS2434::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0xFF), MEM_SIZE);
}

void DS2434::clearScratchpad(void)
{
    memset(scratchpad, static_cast<uint8_t>(0xFF), SCRATCHPAD_SIZE);
}

bool DS2434::writeMemory(const uint8_t* const source, const uint16_t length, const uint16_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);
    return true;
}

bool DS2434::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}

void DS2434::setTemperature(const int8_t temp_degC)
{
    int8_t value = temp_degC;

    if (value > 126) value = 126;
    if (value < -40) value = -40;
    memory[0x61] = value;

    if (value < 0) value = 0;
    uint8_t uvalue = static_cast<uint8_t>(value);

    memory[0x60] = uvalue << 1u;

    // reset request
    request_temp = false;
}

bool DS2434::getTemperatureRequest() const
{
    return (request_temp);
}

void DS2434::lockNV1()
{
    memory[0x62] |= 0b100u;
}

void DS2434::unlockNV1()
{
    memory[0x62] &= ~0b100u;
}

void DS2434::setBatteryCounter(uint16_t value)
{
    *((uint16_t *) &memory[0x82]) = value;
}

void DS2434::setID(uint16_t value)
{
    *((uint16_t *) &memory[0x80]) = value;
}
