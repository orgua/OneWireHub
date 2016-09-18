#include "DS2502.h"

DS2502::DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    clearMemory();
    clearScratchpad();
    clearStatus();
    if (ID1 == 0x11) sizeof_memory = (64 < sizeof(memory)) ? 64 : sizeof(memory); // autorecognize the ds2501 with smaller
    else             sizeof_memory = sizeof(memory);
};

bool DS2502::duty(OneWireHub *hub)
{
    uint16_t reg_TA; // address
    uint8_t  mem_offset = 0;
    uint8_t  b;
    uint8_t  crc = 0;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;
    crc = crc8(&cmd,1,crc);
    
    switch (cmd)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;

            for (int i = 0; i < 32; ++i) // TODO: check for memory_address + 32 < sizeof()
            {
                hub->send(memory[reg_TA + i]);
                if (hub->getError())
                {
                    hub->clearError();
                    break;
                }
            };

            break;

            // READ SCRATCHPAD COMMAND
        case 0xAB:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;

            // Offset
            mem_offset = hub->recv();
            break;

            // READ MEMORY
        case 0xF0:
            b = hub->recv(); // Adr1
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
            crc = crc8(&b,1,crc);

            b = hub->recv(); // Adr2
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;
            crc = crc8(&b,1,crc);

            hub->send(crc);
            if (hub->getError()) break;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof_memory; ++i)
            {
                uint8_t j = translateRedirection(uint8_t(i));
                hub->send(memory[j]);
                if (hub->getError()) break;
                crc = crc8(&memory[j],1,crc);
            };
            if (hub->getError()) break;

            hub->send(crc);
            if (hub->getError()) break;

            while (1) // datasheed says we should return all 1s, send(255), till reset
            {
                hub->send(255);
                if (hub->getError()) break;
            };
            break;

            // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)
        case 0xC3:
            b = hub->recv(); // Adr1
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
            crc = crc8(&b,1,crc);

            b = hub->recv(); // Adr2
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;
            crc = crc8(&b,1,crc);

            hub->send(crc);
            if (hub->getError()) break;

            while (mem_offset < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                mem_offset = uint8_t((reg_TA & ~page_mask) + (1 << 5));
                for (uint16_t i = reg_TA; i < mem_offset; ++i)
                {
                    uint8_t j = translateRedirection(uint8_t(i));
                    hub->send(memory[j]);
                    if (hub->getError()) break;
                    crc = crc8(&memory[j], 1, crc);
                };
                if (hub->getError()) break;

                hub->send(crc);
                if (hub->getError()) break;
                reg_TA = mem_offset;
            };

            while (1) // datasheed says we should return all 1s, send(255), till reset
            {
                hub->send(255);
                if (hub->getError()) break;
            };
            break;

            // READ STATUS
        case 0xAA: // TODO: nearly same code as 0xF0, but with status[] instead of memory[]
            b = hub->recv(); // Adr1
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
            crc = crc8(&b,1,crc);

            b = hub->recv(); // Adr2
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;
            crc = crc8(&b,1,crc);

            hub->send(crc);
            if (hub->getError()) break;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof(status); ++i)
            {
                // TODO: redirection
                hub->send(status[i]);
                if (hub->getError()) break;
                crc = crc8(&status[i],1,crc);
            };
            if (hub->getError()) break;

            hub->send(crc);
            if (hub->getError()) break;

            while (1) // datasheed says we should return all 1s, send(255), till reset
            {
                hub->send(255);
                if (hub->getError()) break;
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
    return !(hub->getError());
};

void DS2502::clearMemory(void)
{
    for (int i = 0; i < sizeof_memory; ++i)  memory[i] = 0x00;
};

void DS2502::clearScratchpad(void)
{
    for (int i = 0; i < sizeof(scratchpad); ++i)  scratchpad[i] = 0x00;
};

void DS2502::clearStatus(void)
{
    for (int i = 0; i < sizeof(status); ++i)  status[i] = 0x00;
};

bool DS2502::writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position)
{
    for (uint8_t i = 0; i < length; ++i)
    {
        if ((position + i) >= sizeof_memory) return false;
        memory[position + i] = source[i];
    };
    return true;
};

bool DS2502::checkProtection(const uint8_t reg_address)
{
    uint8_t reg_index = reg_address >> 5;
    return ((status[0] & (1<<reg_index)) != 0);
};

uint8_t DS2502::translateRedirection(const uint8_t reg_address)
{
    uint8_t reg_index = uint8_t(1) + (reg_address >> 5);

    uint8_t reg_offset = (status[reg_index] == 0xFF) ? reg_address : status[reg_index];

    return ((reg_offset & ~page_mask) | (reg_address & page_mask));
};

bool DS2502::redirectPage(const uint8_t page_source, const uint8_t page_dest)
{
    if (page_source > 3) return false;
    if (page_dest > 3) return false;

    status[page_source + 1] = (page_dest == page_source) ? uint8_t(0xFF) : page_dest << 5;
    return true;
};

bool DS2502::protectPage(const uint8_t page, const bool status_protected)
{
    if (page > 3) return false;

    status[0] &= ~(1<<page);
    status[0] |= (status_protected<<page);

    return true;
};