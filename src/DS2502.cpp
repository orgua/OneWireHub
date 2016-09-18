#include "DS2502.h"

DS2502::DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    if      (ID1 == 0x11)   sizeof_memory = (64 < sizeof(memory)) ? 64 : sizeof(memory); // autorecognize the ds2501 with smaller mem-size
    else if (ID1 == 0x91)   sizeof_memory = (64 < sizeof(memory)) ? 64 : sizeof(memory); // autorecognize the ds2501 with smaller men-size
    else                    sizeof_memory = sizeof(memory);

    clearMemory();
    clearStatus();
};

bool DS2502::duty(OneWireHub *hub)
{
    uint16_t reg_TA; // address
    uint8_t  reg_address = 0;
    uint8_t  b;
    uint8_t  crc = 0;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;
    crc = crc8(&cmd,1,crc);

    b = hub->recv(); // Adr1
    if (hub->getError())  return false;
    reinterpret_cast<uint8_t *>(&reg_TA)[0] = b;
    crc = crc8(&b,1,crc);

    b = hub->recv(); // Adr2
    if (hub->getError())  return false;
    reinterpret_cast<uint8_t *>(&reg_TA)[1] = b;
    crc = crc8(&b,1,crc);
    
    switch (cmd)
    {
        case 0xF0: // READ MEMORY
            hub->send(crc);
            if (hub->getError()) break;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof_memory; ++i)
            {
                uint8_t j = translateRedirection(i);
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

        case 0xC3: // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)
            hub->send(crc);
            if (hub->getError()) break;

            while (reg_address < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                reg_address = uint8_t((reg_TA & ~page_mask) + (1 << 5));
                for (uint16_t i = reg_TA; i < reg_address; ++i)
                {
                    uint8_t j = translateRedirection(i);
                    hub->send(memory[j]);
                    if (hub->getError()) break;
                    crc = crc8(&memory[j], 1, crc);
                };
                if (hub->getError()) break;

                hub->send(crc);
                if (hub->getError()) break;
                reg_TA = reg_address;
            };

            while (1) // datasheed says we should return all 1s, send(255), till reset
            {
                hub->send(255);
                if (hub->getError()) break;
            };
            break;


        case 0xAA: // READ STATUS // TODO: nearly same code as 0xF0, but with status[] instead of memory[]
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

        case 0x0F: // WRITE MEMORY
            if (reg_TA > sizeof_memory) return false; // check for valid address

            b = hub->recv(); // data
            if (hub->getError())  return false;
            crc = crc8(&b,1,crc);

            hub->send(crc);
            if (hub->getError())  return false;
            hub->extendTimeslot();

            if (checkProtection(reg_TA)) return false;

            reg_address = translateRedirection(reg_TA);
            memory[reg_address] &= b; // like EPROM-Mode

            hub->send(memory[reg_address]);
            if (hub->getError())  return false;

            reg_TA++;
            while (reg_TA < sizeof_memory)
            {
                b = hub->recv(); // data
                if (hub->getError())  return false;
                crc = crc8(&b,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                hub->send(crc);
                if (hub->getError())  return false;
                hub->extendTimeslot();

                reg_address = translateRedirection(reg_TA);
                if (!checkProtection(reg_address))
                {
                    memory[reg_address] &= b; // like EPROM-Mode

                    hub->send(memory[reg_address]);
                    if (hub->getError()) return false;
                };
                reg_TA++;
            };
            break;

        case 0x55: // WRITE STATUS
            if (reg_TA > sizeof(status)) return false; // check for valid address

            b = hub->recv(); // data
            if (hub->getError())  return false;
            crc = crc8(&b,1,crc);

            hub->send(crc);
            if (hub->getError())  return false;
            hub->extendTimeslot();

            status[reg_TA] &= b; // like EPROM-Mode

            hub->send(status[reg_TA]);
            if (hub->getError())  return false;

            reg_TA++;
            while (reg_TA < sizeof(status))
            {
                b = hub->recv(); // data
                if (hub->getError())  return false;
                crc = crc8(&b,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                hub->send(crc);
                if (hub->getError())  return false;
                hub->extendTimeslot();

                status[reg_TA] &= b; // like EPROM-Mode
                hub->send(status[reg_TA]);
                if (hub->getError()) return false;

                reg_TA++;
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
    return !(hub->getError());
};

void DS2502::clearMemory(void)
{
    for (uint8_t i = 0; i < sizeof_memory; ++i)  memory[i] = 0xFF;
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