#include "DS2450.h"

DS2450::DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) :
        OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    constexpr uint32_t mem_size = PAGE_COUNT*PAGE_SIZE;
    static_assert(mem_size < 256,  "Implementation does not cover the whole address-space");
    memset(&memory[0], static_cast<uint8_t>(0), mem_size);
    if (mem_size > 0x1C) memory[0x1C] = 0x40;
};

void DS2450::duty(OneWireHub *hub)
{
    uint16_t memory_address;
    //uint16_t memory_address_start; // needed when fully implemented
    uint8_t  b;
    uint16_t crc = 0;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return;

    switch (cmd)
    {
        case 0xAA: // READ MEMORY

            crc = crc16(0xAA, crc); // Cmd

            b = hub->recv(); // Adr1
            if (hub->getError())  return;
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;
            crc = crc16(b, crc);

            b = hub->recv(); // Adr2
            if (hub->getError())  return;
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;
            crc = crc16(b, crc);

            //memory_address_start = memory_address;
            if (memory_address > (PAGE_COUNT-1)*PAGE_SIZE) memory_address = 0; // prevent read out of bounds

            for (uint8_t i = 0; i < PAGE_SIZE; ++i)
            {
                b = memory[memory_address + i];
                if (hub->send(b)) return;
                crc = crc16(b, crc);
            };

            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[0]))  return;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[1]))  return;
            // TODO: not fully implemented

            break;

        case 0x55: // write memory (only page 1&2 allowed)
            crc = crc16(0x55, crc); // Cmd

            b = hub->recv(); // Adr1
            if (hub->getError())  return;
            reinterpret_cast<uint8_t *>(&memory_address)[0] = b;
            crc = crc16(b, crc);

            b = hub->recv(); // Adr2
            if (hub->getError())  return;
            reinterpret_cast<uint8_t *>(&memory_address)[1] = b;
            crc = crc16(b, crc);

            //memory_address_start = memory_address;
            if (memory_address > (PAGE_COUNT-1)*PAGE_SIZE) memory_address = 0; // prevent read out of bounds

            for (uint8_t i = 0; i < PAGE_SIZE; ++i)
            {
                memory[memory_address + i] = hub->recv();
                if (hub->getError()) // possibility to break loop if recv fails
                {
                    hub->clearError();
                    break;
                }
                crc = crc16(memory[memory_address + i], crc);
            };

            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[0]))  return;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[1]))  return;
            // TODO: write back data if wanted, till the end of register

            break;

        case 0x3C: // convert, starts adc
            crc = crc16(0x3C, crc); // Cmd
            crc = crc16(hub->recv(), crc); // input select mask, not important
            if (hub->getError())  return;
            b = hub->recv(); // read out control byte
            if (hub->getError())  return;
            crc = crc16(b, crc);
            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[0]))  return;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc)[1]))  return;

            if (hub->sendBit(false)) return; // still converting....
            if (hub->sendBit(true))  return; // finished conversion
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
};

bool DS2450::setPotentiometer(const uint16_t p1, const uint16_t p2, const uint16_t p3, const uint16_t p4)
{
    setPotentiometer(0, p1);
    setPotentiometer(1, p2);
    setPotentiometer(2, p3);
    setPotentiometer(3, p4);
    return true;
};

bool DS2450::setPotentiometer(const uint8_t number, const uint16_t value)
{
    if (number > 3) return 1;
    uint8_t LByte = static_cast<uint8_t>(value>>0) & static_cast<uint8_t>(0xFF);
    uint8_t HByte = static_cast<uint8_t>(value>>8) & static_cast<uint8_t>(0xFF);
    memory[2*number+0] = LByte;
    memory[2*number+1] = HByte;
    return true;
};
