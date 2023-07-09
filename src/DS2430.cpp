#include "DS2430.h"

DS2430::DS2430(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6,
               uint8_t ID7)
    : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(scratchpad) < 256,
                  "Implementation does not cover the whole address-space");
    static_assert(sizeof(memory) < 256, "Implementation does not cover the whole address-space");

    clearMemory();
    clearScratchpad();
    status_register = 0xFFu;
}

void DS2430::duty(OneWireHub *const hub)
{
    static uint8_t reg_a{0u};

    uint8_t cmd, data;
    if (hub->recv(&cmd, 1u)) return;
    switch (cmd)
    {
        case 0x0F: // WRITE SCRATCHPAD COMMAND
            if (hub->recv(&reg_a, 1u)) return;
            reg_a &= SCRATCHPAD1_MASK;
            while (!hub->recv(&scratchpad[reg_a], 1u))
            {
                reg_a = (reg_a + 1u) & SCRATCHPAD1_MASK;
                // wrap around if at x1F (writes in loop until reset issued)
            }
            break;

        case 0xAA: // READ SCRATCHPAD COMMAND
            if (hub->recv(&reg_a, 1u)) return;
            reg_a &= SCRATCHPAD1_MASK;
            while (!hub->send(&scratchpad[reg_a], 1u))
            {
                reg_a = (reg_a + 1u) & SCRATCHPAD1_MASK;
                // wrap around if at x1F (reads in loop until reset issued)
            }
            break;

        case 0x55: // COPY SCRATCHPAD COMMAND
            if (hub->recv(&data)) return;
            if (data != 0xA5u) break; // verification
            writeMemory(scratchpad, SCRATCHPAD1_SIZE, 0u);
            break;

        case 0xF0: // READ MEMORY COMMAND
            if (hub->recv(&reg_a, 1u)) return;
            reg_a &= SCRATCHPAD1_MASK;
            while (!hub->send(&memory[reg_a], 1u))
            {
                reg_a = (reg_a + 1u) & SCRATCHPAD1_MASK;
                // wrap around if at x1F (reads in loop until reset issued)
            }
            break;

        case 0x99: // WRITE APPLICATION REGISTER
            if (hub->recv(&reg_a, 1u)) return;
            reg_a = (reg_a & SCRATCHPAD2_MASK) + SCRATCHPAD2_ADDR;
            if (!(status_register & 0b11)) return;
            while (!hub->recv(&scratchpad[reg_a], 1u))
            {
                reg_a = ((reg_a + 1u) & SCRATCHPAD2_MASK) + SCRATCHPAD2_ADDR;
                // wrap around if at x07 (writes in loop until reset issued)
            }
            break;

        case 0x66: // READ STATUS REGISTER
            if (hub->recv(&reg_a)) return;
            if (data != 0x00u) break; // verification
            hub->send(&status_register, 1u);
            break;

        case 0xC3: // READ APPLICATION REGISTER COMMAND
            if (hub->recv(&reg_a, 1u)) return;
            reg_a = (reg_a & SCRATCHPAD2_MASK) + SCRATCHPAD2_ADDR;
            // original IC distinguishes between MEM and scratchpad here, but content is the same
            while (!hub->send(&scratchpad[reg_a], 1u))
            {
                reg_a = ((reg_a + 1u) & SCRATCHPAD2_MASK) + SCRATCHPAD2_ADDR;
                // wrap around if at x07 (writes in loop until reset issued)
            }
            break;

        case 0x5A: // COPY & LOCK APPLICATION REGISTER
            if (hub->recv(&data)) return;
            if (data != 0xA5u) break; // verification
            if (!(status_register & 0b11)) return;
            writeMemory(&scratchpad[SCRATCHPAD2_ADDR], SCRATCHPAD2_SIZE, SCRATCHPAD2_ADDR);
            status_register &= ~0b11u; // lock the OTP
            break;

        default: hub->raiseDeviceError(cmd);
    }
}

void DS2430::clearMemory(void) { memset(memory, static_cast<uint8_t>(0x00), sizeof(memory)); }

void DS2430::clearScratchpad(void)
{
    memset(scratchpad, static_cast<uint8_t>(0x00), SCRATCHPAD_SIZE);
}

bool DS2430::writeMemory(const uint8_t *const source, const uint8_t length, const uint8_t position)
{
    for (uint8_t i = 0; i < length; ++i)
    {
        if ((position + i) >= sizeof(memory)) break;
        memory[position + i] = source[position + i];
    }
    return true;
}

bool DS2430::readMemory(uint8_t *const destination, const uint16_t length,
                        const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination, &memory[position], _length);
    return (_length == length);
}

bool DS2430::syncScratchpad()
{
    uint8_t length = (MEM_SIZE > SCRATCHPAD_SIZE) ? SCRATCHPAD_SIZE : MEM_SIZE;
    memcpy(scratchpad, memory, length);
    return true;
}
