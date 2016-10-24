#include "DS2502.h"

DS2502::DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 256,  "Implementation does not cover the whole address-space");

    if      (ID1 == 0x11)   sizeof_memory = (64 < sizeof(memory)) ? 64 : sizeof(memory); // autorecognize the ds2501 with smaller mem-size
    else if (ID1 == 0x91)   sizeof_memory = (64 < sizeof(memory)) ? 64 : sizeof(memory); // autorecognize the ds2501 with smaller men-size
    else                    sizeof_memory = sizeof(memory);

    clearMemory();
    clearStatus();
};

void DS2502::duty(OneWireHub *hub)
{
    uint16_t reg_TA; // address
    uint8_t  reg_address = 0;
    uint8_t  b;
    uint8_t  crc = 0;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return;
    crc = crc8(&cmd,1,crc);

    b = hub->recv(); // Adr1
    if (hub->getError())  return;
    reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
    crc = crc8(&b,1,crc);

    b = hub->recv(); // Adr2
    if (hub->getError())  return;
    reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;
    crc = crc8(&b,1,crc);
    
    switch (cmd)
    {
        case 0xF0:      // READ MEMORY
            if (hub->send(crc)) return;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof_memory; ++i)
            {
                uint8_t j = translateRedirection(i);
                if (hub->send(memory[j])) return;
                crc = crc8(&memory[j],1,crc);
            };

            hub->send(crc);
            // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive
            return;

        case 0xC3:      // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)
            if (hub->send(crc)) return;

            while (reg_address < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                reg_address = uint8_t((reg_TA & ~page_mask) + (1 << 5));
                for (uint16_t i = reg_TA; i < reg_address; ++i)
                {
                    uint8_t j = translateRedirection(i);
                    if (hub->send(memory[j])) return;
                    crc = crc8(&memory[j], 1, crc);
                };

                if (hub->send(crc)) return;
                reg_TA = reg_address;
            };

            // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive
            return;

        case 0xAA:      // READ STATUS // TODO: nearly same code as 0xF0, but with status[] instead of memory[]
            if (hub->send(crc)) return;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof(status); ++i)
            {
                // TODO: redirection
                if (hub->send(status[i])) return;
                crc = crc8(&status[i],1,crc);
            };

            hub->send(crc);
            // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive
            return;

        case 0x0F:      // WRITE MEMORY
            if (reg_TA > sizeof_memory) return; // check for valid address

            b = hub->recv(); // data
            if (hub->getError())  return;
            crc = crc8(&b,1,crc);

            if (hub->send(crc))   return;
            hub->extendTimeslot();

            if (checkProtection(reg_TA)) return;

            reg_address = translateRedirection(reg_TA);
            memory[reg_address] &= b; // like EPROM-Mode

            if (hub->send(memory[reg_address])) return;

            reg_TA++;
            while (reg_TA < sizeof_memory)
            {
                b = hub->recv(); // data
                if (hub->getError())  return;
                crc = crc8(&b,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                if (hub->send(crc))     return;
                hub->extendTimeslot();

                reg_address = translateRedirection(reg_TA);
                if (!checkProtection(reg_address))
                {
                    memory[reg_address] &= b; // like EPROM-Mode

                    if (hub->send(memory[reg_address])) return;
                };
                reg_TA++;
            };
            return;

        case 0x55:      // WRITE STATUS
            if (reg_TA > sizeof(status)) return; // check for valid address

            b = hub->recv(); // data
            if (hub->getError())  return;
            crc = crc8(&b,1,crc);

            if (hub->send(crc))  return;
            hub->extendTimeslot();

            status[reg_TA] &= b; // like EPROM-Mode

            if (hub->send(status[reg_TA]))  return;

            reg_TA++;
            while (reg_TA < sizeof(status))
            {
                b = hub->recv(); // data
                if (hub->getError())  return;
                crc = crc8(&b,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                if (hub->send(crc))  return;
                hub->extendTimeslot();

                status[reg_TA] &= b; // like EPROM-Mode
                if (hub->send(status[reg_TA])) return;

                reg_TA++;
            };
            return;

        default:
            hub->raiseSlaveError(cmd);
    };
};

void DS2502::clearMemory(void)
{
    memset(&memory[0], static_cast<uint8_t>(0xFF), sizeof_memory);
};

void DS2502::clearStatus(void)
{
    for (uint8_t i = 0; i < sizeof(status); ++i)  status[i] = 0xFF;
    status[sizeof(status)-1] = 0x00; // last byte should be always zero
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

bool DS2502::checkProtection(const uint16_t reg_address)
{
    uint8_t reg_index = uint8_t(reg_address >> 5);
    return ((status[0] & (1<<reg_index)) == 0);
};

uint8_t DS2502::translateRedirection(const uint16_t reg_address)
{
    uint8_t reg_index = uint8_t(1) + uint8_t(reg_address >> 5);

    uint8_t reg_offset = (status[reg_index] == 0xFF) ? uint8_t(reg_address) : ((~status[reg_index])<<5);

    return ((reg_offset & ~page_mask) | uint8_t(reg_address & page_mask));
};

bool DS2502::redirectPage(const uint8_t page_source, const uint8_t page_dest)
{
    if (page_source > 3) return false;
    if (page_dest > 3) return false;

    status[page_source + 1] = (page_dest == page_source) ? uint8_t(0xFF) : ~page_dest; // datasheet dictates this, so no page can be redirected to page 0
    return true;
};

bool DS2502::protectPage(const uint8_t page, const bool status_protected)
{
    if (page > 3) return false;

    status[0] &= ~(1<<page);
    status[0] |= ((!status_protected)&0x01)<<page;

    return true;
};