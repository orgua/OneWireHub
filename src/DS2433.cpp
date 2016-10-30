#include "DS2433.h"

DS2433::DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 65535,  "Implementation does not cover the whole address-space");
    clearMemory();
};

void DS2433::duty(OneWireHub *hub)
{
    constexpr uint8_t ALTERNATE_01 = 0b10101010;

    static uint16_t reg_TA; // contains TA1, TA2
    static uint8_t  reg_ES = 31;  // E/S register

    uint8_t  mem_counter = 0; // offer feedback with PF-bit
    uint16_t crc = 0;
    uint8_t  b;

    uint8_t cmd = 0;
    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0x0F:      // WRITE SCRATCHPAD COMMAND
            // Adr1
            if (hub->recv(&b,1,crc)) return;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
            reg_ES = b & uint8_t(0b00011111); // register-offset
            // Adr2
            if (hub->recv(&b,1,crc)) return;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b & uint8_t(0b1);

            mem_counter = static_cast<uint8_t>(32-reg_ES);
            if (hub->recv(&memory[reg_TA],mem_counter,crc)) return;

            reg_ES = 0b00011111;

            crc = ~crc; // normally crc16 is sent ~inverted
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2))  return;
            break;

        case 0x55:      // COPY SCRATCHPAD
            if (hub->recv(&b)) return; // TA1
            //if (b != reinterpret_cast<uint8_t *>(&reg_TA)[0]) return;

            if (hub->recv(&b)) return;  // TA2
            //if (b != reinterpret_cast<uint8_t *>(&reg_TA)[1]) return;

            if (hub->recv(&b)) return;  // ES
            //if (b != reg_ES) return;

            reg_ES |= 0b10000000;

            delayMicroseconds(5000); // simulate writing

            hub->sendBit(true);
            hub->clearError();

            while (true) // send alternating 1 & 0 after copy is complete
            {
                if (hub->send(&ALTERNATE_01)) return;
            };

        case 0xAA:      // READ SCRATCHPAD COMMAND

            if (hub->send(reinterpret_cast<uint8_t *>(&reg_TA),2))  return; // Adr1

            if (hub->send(&reg_ES)) return; // ES

            // TODO: maybe implement a real scratchpad, would need 32byte extra ram

            // data
            if (hub->send(&memory[(reg_TA & ~uint16_t(0b00011111))],32)) return;

            // datasheed says we should send all 1s, till reset (1s are passive... so nothing to do here)
            return;

        case 0xF0:      // READ MEMORY
            // Adr1
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2)) return;

            // data
            for (uint16_t i = reg_TA; i < 512; i+=32) // model of the 32byte scratchpad
            {
                if (hub->send(&memory[i],32)) return;
            };

            // datasheed says we should send all 1s, till reset (1s are passive... so nothing to do here)
            return;

        default:
            hub->raiseSlaveError(cmd);
    };
};

void DS2433::clearMemory(void)
{
    memset(&memory[0], static_cast<uint8_t>(0x00), sizeof(memory));
};

bool DS2433::writeMemory(const uint8_t* source, const uint16_t length, const uint16_t position)
{
    for (uint16_t i = 0; i < length; ++i)
    {
        if ((position + i) >= sizeof(memory)) return false;
        memory[position + i] = source[i];
    };
    return true;
};
