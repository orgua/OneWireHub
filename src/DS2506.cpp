#include "DS2506.h"

DS2506::DS2506(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) <= 0xFFFF, "Implementation does not cover the whole address-space");

    // set device specific "real" sizes
    switch (ID1)
    {
        case 0x13:  // DS2503
            sizeof_memory = 512;
            break;

        case 0x0B:  // DS2505
            sizeof_memory = 2048;
            break;

        case 0x0F:  // DS2506
            sizeof_memory = 8192;
            break;

        default:
            sizeof_memory = 256;
    }

    page_count      = sizeof_memory / PAGE_SIZE;                // DS2506: 256
    status_segment  = page_count / uint8_t(8);                  // DS2506:  32

    clearMemory();
    clearStatus();
}

void DS2506::duty(OneWireHub * const hub)
{
    uint16_t reg_TA, reg_RA = 0, crc = 0; // Target address
    uint8_t  cmd, data; // redirected address, command, data, crc

    // always receives cmd and TA
    if (hub->recv(&cmd,1,crc))  return;
    if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc))  return;

    switch (cmd)
    {
        case 0xF0:      // READ MEMORY

            while (reg_TA <= sizeof_memory)
            {
                const uint16_t destin_TA = translateRedirection(reg_TA);
                const uint8_t  length    = PAGE_SIZE - uint8_t(reg_TA & PAGE_MASK);

                if (destin_TA < MEM_SIZE)
                {
                    if (hub->send(&memory[destin_TA],length,crc)) return;
                }
                else // fake data
                {
                    data  = 0x00;
                    uint8_t counter = length;
                    while (counter-- > 0)
                    {
                        if (hub->send(&data, 1, crc)) return;
                    }
                }

                reg_TA += length;
            }
            crc = ~crc; // normally crc16 is sent ~inverted
            hub->send(reinterpret_cast<uint8_t *>(&crc),2);
            break; // datasheet says we should return 1s, till reset, nothing to do here

        case 0xA5:      // EXTENDED READ MEMORY (with redirection-information)

            while (reg_TA <= sizeof_memory)
            {
                const uint8_t  source_page = static_cast<uint8_t>(reg_TA>>5);
                const uint8_t  destin_page = getPageRedirection(source_page);
                if (hub->send(&destin_page,1,crc))                        return;
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc),2); // send crc of (cmd,TA,destin_page) at first, then only crc of (destin_page)
                crc=0;
                const uint16_t destin_TA   = translateRedirection(reg_TA);
                const uint8_t  length      = PAGE_SIZE - uint8_t(reg_TA & PAGE_MASK);

                if (destin_TA < MEM_SIZE)
                {
                    if (hub->send(&memory[destin_TA],length,crc)) return;
                }
                else // fake data
                {
                    data  = 0x00;
                    uint8_t counter = length;
                    while (counter-- != 0)
                    {
                        if (hub->send(&data, 1, crc)) return;
                    }
                }

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) break;
                reg_TA += length;
                crc=0;
            }
            break; // datasheet says we should return 1s, till reset, nothing to do here

        case 0xAA:      // READ STATUS

            while (reg_TA < STATUS_SIZE_DEV) // check for valid address
            {
                reg_RA = reg_TA&uint8_t(7);
                while (reg_RA < 8)
                {
                    data = readStatus(reg_TA); // read byte by byte
                    if (hub->send(&data, 1, crc)) return;
                    reg_RA++;
                    reg_TA++;
                }
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc),2);
                crc = 0;
            }
            break;

        case 0x0F:      // WRITE MEMORY

            while (reg_TA < sizeof_memory) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) break;
                // master issues now a 480us 12V-Programming Pulse -> advantage for us, enough time to handle addressMapping

                reg_RA = translateRedirection(reg_TA);
                const uint8_t  page = static_cast<uint8_t>(reg_RA>>5);
                if (getPageProtection(page))
                {
                    const uint8_t mem_zero = 0x00;
                    if (hub->send(&mem_zero)) break;
                }
                else
                {
                    memory[reg_RA] &= data; // like EEPROM-Mode
                    setPageUsed(page);
                    if (hub->send(&memory[reg_RA])) break;
                }
                crc = ++reg_TA; // prepare new loop
            }
            break;

        case 0xF3:      // SPEED WRITE MEMORY, omit CRC

            while (reg_TA < sizeof_memory) // check for valid address
            {
                if (hub->recv(&data)) break;
                // master issues now a 480us 12V-Programming Pulse

                reg_RA = translateRedirection(reg_TA);
                const uint8_t  page = static_cast<uint8_t>(reg_RA>>5);
                if (getPageProtection(page))
                {
                    const uint8_t mem_zero = 0x00;
                    if (hub->send(&mem_zero)) break;
                }
                else
                {
                    memory[reg_RA] &= data; // like EEPROM-Mode
                    setPageUsed(page);
                    if (hub->send(&memory[reg_RA])) break;
                }
                ++reg_TA; // prepare new loop
            }
            break;

        case 0x55:      // WRITE STATUS

            while (reg_TA < STATUS_SIZE_DEV) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) break;
                // master issues now a 480us 12V-Programming Pulse

                data = writeStatus(reg_TA, data);
                if (hub->send(&data)) break;
                crc = ++reg_TA; // prepare new loop
            }
            break;

        case 0xF5:      // SPEED WRITE STATUS, omit CRC

            while (reg_TA < STATUS_SIZE_DEV) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;
                // master issues now a 480us 12V-Programming Pulse

                data = writeStatus(reg_TA, data);
                if (hub->send(&data)) break;
                ++reg_TA; // prepare new loop
            }
            break;

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2506::clearMemory(void)
{
    memset(memory, value_xFF, MEM_SIZE);
}

void DS2506::clearStatus(void)
{
    memset(status, value_xFF, STATUS_SIZE);
}

bool DS2506::writeMemory(const uint8_t* const source, const uint16_t length, const uint16_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);

    const uint8_t page_start = static_cast<uint8_t>(position >> 5);
    const uint8_t page_stop  = static_cast<uint8_t>((position + _length) >> 5);
    for (uint8_t page = page_start; page <= page_stop; page++) setPageUsed(page);

    return (_length==length);
}

bool DS2506::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
}

uint16_t DS2506::translateRedirection(const uint16_t source_address) const// TODO: extended read mem description implies that redirection is recursive
{
    const uint8_t  source_page    = static_cast<uint8_t >(source_address >> 5);
    const uint8_t  destin_page    = getPageRedirection(source_page);
    if (destin_page == 0x00)        return source_address;
    const uint16_t destin_address = (source_address & PAGE_MASK) | (destin_page << 5);
    return destin_address;
}


uint8_t DS2506::readStatus(const uint16_t address) const
{
    uint16_t SA = address;
    uint8_t  return_value { 0x00 };

    if (address < STATUS_WP_REDIR_BEG)                              // is WP_PAGES
    {
        if (SA < STATUS_SEGMENT) return_value = status[SA+0*STATUS_SEGMENT]; // emulate protection
    }
    else if (address < STATUS_PG_WRITN_BEG)                         // is WP_REDIR
    {
        SA -= STATUS_WP_REDIR_BEG;
        if (SA < STATUS_SEGMENT) return_value = status[SA+1*STATUS_SEGMENT]; // emulate protection
    }
    else if (address < STATUS_UNDEF_B1_BEG)                         // is PG_WRITTEN
    {
        SA -= STATUS_PG_WRITN_BEG;
        if (SA < STATUS_SEGMENT) return_value = status[SA+2*STATUS_SEGMENT]; // emulate written
    }
    else if (address < STATUS_PG_REDIR_BEG)                         // is undefined
    {
        return_value = 0xFF;                                        // emulate undefined read
    }
    else if (address < STATUS_UNDEF_B2_BEG)                         // is PG_REDIRECTION
    {
        SA -= STATUS_PG_REDIR_BEG;
        if (SA < PAGE_COUNT)    return_value = status[SA+3*STATUS_SEGMENT];
        else                    return_value = 0xFF;                // emulate no redirection
    }
    else return_value = 0xFF;                                       // is undefined

    return return_value;
}

uint8_t DS2506::writeStatus(const uint16_t address, const uint8_t value)
{
    uint16_t SA = address;

    if (address < STATUS_WP_REDIR_BEG)              // is WP_PAGES
    {
        if (SA >= STATUS_SEGMENT) return 0x00;      // emulate protection
    }
    else if (address < STATUS_PG_WRITN_BEG)         // is WP_REDIR
    {
        SA -= STATUS_WP_REDIR_BEG;
        if (SA >= STATUS_SEGMENT) return 0x00;      // emulate protection
        SA += STATUS_SEGMENT;
    }
    else if (address < STATUS_UNDEF_B1_BEG)         // is PG_WRITTEN
    {
        SA -= STATUS_PG_WRITN_BEG;
        if (SA >= STATUS_SEGMENT) return 0x00;      // emulate written
        SA += 2*STATUS_SEGMENT;
    }
    else if (address < STATUS_PG_REDIR_BEG)         // is undefined
    {
        return 0xFF;                                // emulate undefined read
    }
    else if (address < STATUS_UNDEF_B2_BEG)         // is PG_REDIRECTION
    {
        SA -= STATUS_PG_REDIR_BEG;
        if (SA >= PAGE_COUNT)    return 0xFF;       // emulate no redirection
        if (getRedirectionProtection(static_cast<uint8_t>(SA))) return 0x00;
        SA += 3*STATUS_SEGMENT;
    }
    else return 0xFF;                               // is undefined

    status[SA] &= value;
    return status[SA];
}

void DS2506::setPageProtection(const uint8_t page)
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return;
    const uint8_t page_mask = ~(uint8_t(1)<<(page&7));
    status[segment_pos] &= page_mask;
}

bool DS2506::getPageProtection(const uint8_t page) const
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return true;
    const uint8_t page_mask = (uint8_t(1)<<(page&7));
    return ((status[segment_pos] & page_mask) == 0);
}

void DS2506::setRedirectionProtection(const uint8_t page)
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return;
    const uint8_t page_mask = ~(uint8_t(1)<<(page&7));
    status[STATUS_SEGMENT + segment_pos] &= page_mask;
}

bool DS2506::getRedirectionProtection(const uint8_t page) const
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return true;
    const uint8_t page_mask = (uint8_t(1)<<(page&7));
    return ((status[STATUS_SEGMENT + segment_pos] & page_mask) == 0);
}

void DS2506::setPageUsed(const uint8_t page)
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return;
    const uint8_t page_mask = ~(uint8_t(1)<<(page&7));
    status[2*STATUS_SEGMENT + segment_pos] &= page_mask;
}

bool DS2506::getPageUsed(const uint8_t page) const
{
    const uint8_t segment_pos = (page>>3);
    if (segment_pos >= STATUS_SEGMENT) return true;
    const uint8_t page_mask = (uint8_t(1)<<(page&7));
    return ((status[2*STATUS_SEGMENT + segment_pos] & page_mask) == 0);
}

bool DS2506::setPageRedirection(const uint8_t page_source, const uint8_t page_destin)
{
    if (page_source >= PAGE_COUNT)  return false; // really available
    if (page_destin >= page_count)  return false; // virtual mem of the device
    if (getRedirectionProtection(page_source)) return false;

    status[3*STATUS_SEGMENT + page_source] = (page_destin == page_source) ? uint8_t(0xFF) : ~page_destin; // datasheet dictates this, so no page can be redirected to page 0
    return true;
}

uint8_t DS2506::getPageRedirection(const uint8_t page) const
{
    if (page >= PAGE_COUNT) return 0x00;
    return (~status[3*STATUS_SEGMENT + page]); // TODO: maybe invert this in ReadStatus and safe some Operations? Redirection is critical and often done
}
