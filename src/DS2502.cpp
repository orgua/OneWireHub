#include "DS2502.h"

DS2502::DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 256, "Implementation does not cover the whole address-space");

    switch (ID1)
    {
        case 0x11:  // DS2501
        case 0x91:
            sizeof_memory = 64;
            break;

        case 0x09:  // DS2502
        case 0x89:
            sizeof_memory = 128;
            break;

        default:
            sizeof_memory = 64;
    };

    clearMemory();
    clearStatus();
};

void DS2502::duty(OneWireHub *hub)
{
    uint16_t reg_TA, reg_RA = 0; // Target address
    uint8_t  cmd, data, crc = 0; // redirected address, command, data, crc

    if (hub->recv(&cmd))  return;
    crc = crc8(&cmd,1,crc);

    if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2))  return;
    crc = crc8(reinterpret_cast<uint8_t *>(&reg_TA),2,crc);

    switch (cmd)
    {
        case 0xF0:      // READ MEMORY
            if (hub->send(&crc))        break;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof_memory; ++i)
            {
                uint8_t j = translateRedirection(i);
                if (hub->send(&memory[j])) return;
                crc = crc8(&memory[j],1,crc);
            };
            hub->send(&crc);
            break; // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive

        case 0xC3:      // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)
            if (hub->send(&crc)) break;

            while (reg_RA < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                reg_RA = uint16_t((reg_TA & ~PAGE_MASK) + (1 << 5));
                for (uint16_t i = reg_TA; i < reg_RA; ++i)
                {
                    uint8_t j = translateRedirection(i);
                    if (hub->send(&memory[j])) return;
                    crc = crc8(&memory[j], 1, crc);
                };

                if (hub->send(&crc)) break;
                reg_TA = reg_RA;
            };
            break; // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive

        case 0xAA:      // READ STATUS // TODO: nearly same code as 0xF0, but with status[] instead of memory[]
            if (hub->send(&crc)) break;

            crc = 0; // reInit CRC and send data
            for (uint16_t i = reg_TA; i < sizeof(status); ++i)
            {
                // TODO: redirection
                if (hub->send(&status[i])) return;
                crc = crc8(&status[i],1,crc);
            };

            hub->send(&crc);
            // datasheed says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive
            break;

        case 0x0F:      // WRITE MEMORY
            if (reg_TA > sizeof_memory)     break; // check for valid address

            if (hub->recv(&data))           break;
            crc = crc8(&data,1,crc);

            if (hub->send(&crc))            break;

            if (checkProtection(reg_TA))    break;

            reg_RA = translateRedirection(reg_TA);
            memory[reg_RA] &= data; // like EPROM-Mode

            if (hub->send(&memory[reg_RA])) break;

            reg_TA++;
            while (reg_TA < sizeof_memory)
            {
                if (hub->recv(&data))       break;
                crc = crc8(&data,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                if (hub->send(&crc))        break;

                reg_RA = translateRedirection(reg_TA);
                if (!checkProtection(reg_RA))
                {
                    memory[reg_RA] &= data; // like EPROM-Mode
                    if (hub->send(&memory[reg_RA])) break;
                };
                reg_TA++;
            };
            break;

        case 0x55:      // WRITE STATUS
            if (reg_TA > sizeof(status))    break; // check for valid address

            if (hub->recv(&data))           break;
            crc = crc8(&data,1,crc);

            if (hub->send(&crc))            break;

            status[reg_TA] &= data; // like EPROM-Mode

            if (hub->send(&status[reg_TA])) break;

            reg_TA++;
            while (reg_TA < sizeof(status))
            {
                if (hub->recv(&data))       break;
                crc = crc8(&data,1,reinterpret_cast<uint8_t *>(&reg_TA)[0]);

                if (hub->send(&crc))        break;

                status[reg_TA] &= data; // like EPROM-Mode
                if (hub->send(&status[reg_TA])) break;

                reg_TA++;
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
};

void DS2502::clearMemory(void)
{
    memset(&memory[0], static_cast<uint8_t>(0xFF), SIZE_MEM);
};

void DS2502::clearStatus(void)
{
    for (uint8_t i = 0; i < sizeof(status); ++i)  status[i] = 0xFF;
    status[sizeof(status)-1] = 0x00; // last byte should be always zero
};

bool DS2502::writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position)
{
    const uint16_t _length = (position + length >= SIZE_MEM) ? (SIZE_MEM - position) : length; // TODO: dirty hack, just changed sizeofmem to MEM_SIZE
    memcpy(&memory[position],source,_length);
    return (_length==length);
};

bool DS2502::checkProtection(const uint16_t reg_address)
{
    uint8_t reg_index = uint8_t(reg_address >> 5);
    return ((status[0] & (1<<reg_index)) == 0);
};

uint8_t DS2502::translateRedirection(const uint16_t reg_address)
{
    if (reg_address >= SIZE_MEM) return (SIZE_MEM-1); // out of bound is translated to last byte
    uint8_t reg_index = uint8_t(1) + uint8_t(reg_address >> 5);
    uint8_t reg_offset = (status[reg_index] == 0xFF) ? uint8_t(reg_address) : ((~status[reg_index])<<5);
    return ((reg_offset & ~PAGE_MASK) | uint8_t(reg_address & PAGE_MASK));
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
