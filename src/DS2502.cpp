#include "DS2502.h"

DS2502::DS2502(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(MEM_SIZE < 256, "Implementation does not cover the whole address-space");

    clearMemory();
    clearStatus();

    if ((ID1 == 0x11) || (ID1 == 0x91))
    {
        // when set to DS2501, the upper two memory pages are not accessible, always read 0xFF
        for (uint8_t page = 2; page < PAGE_COUNT; ++page)
        {
            setPageUsed(page);
            setPageProtection(page);
            sizeof_memory = 64;
        }
    }
    else
    {
        sizeof_memory = 128;    // must be DS2502 then
    }
}

void DS2502::duty(OneWireHub * const hub)
{
    uint8_t  reg_TA[2], cmd, data, crc = 0; // Target address, redirected address, command, data, crc

    if (hub->recv(&cmd))  return;
    crc = crc8(&cmd,1,crc);

    if (hub->recv(reg_TA,2))  return;
    crc = crc8(reg_TA,2,crc);

    if (reg_TA[1] != 0) return; // upper byte of target adress should not contain any data

    switch (cmd)
    {
        case 0xF0:      // READ MEMORY

            if (hub->send(&crc))        break;

            crc = 0; // reInit CRC and send data
            for (uint8_t i = reg_TA[0]; i < sizeof_memory; ++i)
            {
                const uint8_t reg_RA = translateRedirection(i);
                if (hub->send(&memory[reg_RA])) return;
                crc = crc8(&memory[reg_RA],1,crc);
            }
            hub->send(&crc);
            break; // datasheet says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive

        case 0xC3:      // READ DATA (like 0xF0, but repeatedly till the end of page with following CRC)

            if (hub->send(&crc)) break;

            while (reg_TA[0] < sizeof_memory)
            {
                crc = 0; // reInit CRC and send data
                const uint8_t reg_EA = (reg_TA[0] & ~PAGE_MASK) + PAGE_SIZE; // End Address
                for (uint8_t i = reg_TA[0]; i < reg_EA; ++i)
                {
                    const uint8_t reg_RA = translateRedirection(i);
                    if (hub->send(&memory[reg_RA])) return;
                    crc = crc8(&memory[reg_RA], 1, crc);
                }

                if (hub->send(&crc)) break;
                reg_TA[0] = reg_EA;
            }
            break; // datasheet says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive

        case 0xAA:      // READ STATUS // TODO: nearly same code as 0xF0, but with status[] instead of memory[]

            if (hub->send(&crc)) break;

            crc = 0; // reInit CRC and send data
            for (uint8_t i = reg_TA[0]; i < STATUS_SIZE; ++i)
            {
                if (hub->send(&status[i])) return;
                crc = crc8(&status[i],1,crc);
            }
            hub->send(&crc);
            break; // datasheet says we should return all 1s, send(255), till reset, nothing to do here, 1s are passive

        case 0x0F:      // WRITE MEMORY

            while (reg_TA[0] < sizeof_memory)
            {
                if (hub->recv(&data))       break;
                crc = crc8(&data,1,crc);

                if (hub->send(&crc))        break;

                const uint8_t reg_RA = translateRedirection(reg_TA[0]);

                if (getPageProtection(reg_TA[0]))
                {
                    const uint8_t mem_zero = 0x00; // send dummy data
                    if (hub->send(&mem_zero)) break;
                }
                else
                {
                    memory[reg_RA] &= data; // EPROM-Mode
                    setPageUsed(reg_RA);
                    if (hub->send(&memory[reg_RA])) break;
                }
                crc = ++reg_TA[0];
            }
            break;

        case 0x55:      // WRITE STATUS

            while (reg_TA[0] < STATUS_SIZE)
            {
                if (hub->recv(&data))       break;
                crc = crc8(&data,1,crc);

                if (hub->send(&crc))        break;

                data = writeStatus(reg_TA[0], data);

                if (hub->send(&data)) break;

                crc = ++reg_TA[0];
            }
            break;

        default:

            hub->raiseSlaveError(cmd);
    }
}

uint8_t DS2502::translateRedirection(const uint8_t source_address) const
{
    const uint8_t  source_page    = static_cast<uint8_t >(source_address >> 5);
    const uint8_t  destin_page    = getPageRedirection(source_page);
    if (destin_page == 0x00)        return source_address;
    const uint8_t destin_address  = (source_address & PAGE_MASK) | (destin_page << 5);
    return destin_address;
}

void DS2502::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0xFF), MEM_SIZE);
}

void DS2502::clearStatus(void)
{
    memset(status, static_cast<uint8_t>(0xFF), STATUS_SIZE);
    status[STATUS_FACTORYP] = 0x00; // last byte should be always zero
}

bool DS2502::writeMemory(const uint8_t* const source, const uint8_t length, const uint8_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);

    const uint8_t page_start = static_cast<uint8_t>(position >> 5);
    const uint8_t page_stop  = static_cast<uint8_t>((position + _length) >> 5);
    for (uint8_t page = page_start; page <= page_stop; page++) setPageUsed(page);

    return (_length==length);
}

bool DS2502::readMemory(uint8_t* const destination, const uint8_t length, const uint8_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}


uint8_t DS2502::writeStatus(const uint8_t address, const uint8_t value)
{
    if (address < STATUS_UNDEF_B1)  status[address] &= value; // writing is allowed only here
    return status[address];
}

uint8_t DS2502::readStatus(const uint8_t address) const
{
    if (address >= STATUS_SIZE)     return 0xFF;
    return status[address];
}


void DS2502::setPageProtection(const uint8_t page)
{
    if (page < PAGE_COUNT)          status[STATUS_WP_PAGES] &= ~(uint8_t(1<<page));
}

bool DS2502::getPageProtection(const uint8_t page) const
{
    if (page >= PAGE_COUNT) return true;
    return ((status[STATUS_WP_PAGES] & uint8_t(1<<page)) == 0);
}

void DS2502::setPageUsed(const uint8_t page)
{
    if (page < PAGE_COUNT)  status[STATUS_WP_PAGES] &= ~(uint8_t(1<<(page+4)));
}

bool DS2502::getPageUsed(const uint8_t page) const
{
    if (page >= PAGE_COUNT) return true;
    return ((status[STATUS_WP_PAGES] & uint8_t(1<<(page+4))) == 0);
}


bool DS2502::setPageRedirection(const uint8_t page_source, const uint8_t page_destin)
{
    if (page_source >= PAGE_COUNT)  return false; // really available
    if (page_destin >= PAGE_COUNT)  return false; // virtual mem of the device

    status[page_source + STATUS_PG_REDIR] = (page_destin == page_source) ? uint8_t(0xFF) : ~page_destin; // datasheet dictates this, so no page can be redirected to page 0
    return true;
}

uint8_t DS2502::getPageRedirection(const uint8_t page) const
{
    if (page >= PAGE_COUNT) return 0x00;
    return ~(status[page + STATUS_PG_REDIR]); // TODO: maybe invert this in ReadStatus and safe some Operations? Redirection is critical and often done
}
