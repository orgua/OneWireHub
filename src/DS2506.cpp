#include "DS2506.h"

DS2506::DS2506(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) <= 0xFFFF, "Implementation does not cover the whole address-space");

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
    };

    page_count      = sizeof_memory / PAGE_SIZE;                // DS2506: 256
    status_segment  = page_count / uint8_t(8);                  // DS2506:  32
    sizeof_status   = page_count + (uint8_t(3)*status_segment); // DS2506: 352

    clearMemory();
    clearStatus();
};

void DS2506::duty(OneWireHub *hub)
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
                const uint16_t real_TA = translateRedirection(reg_TA); // TODO: seems to be without redirection? check
                const uint8_t  length  = PAGE_SIZE - uint8_t(reg_TA & PAGE_MASK);
                if (hub->send(&memory[real_TA],length,crc)) return;
                reg_TA += length;
            };
            crc = ~crc; // normally crc16 is sent ~inverted
            hub->send(reinterpret_cast<uint8_t *>(&crc),2);
            break; // datasheed says we should return 1s, till reset, nothing to do here

        case 0xA5:      // EXTENDED READ MEMORY (with redirection-information)
            while (reg_TA <= sizeof_memory)
            {
                const uint8_t  redir   = getRedirection(reg_TA);
                if (hub->send(&redir,1,crc))                        return;
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc),2); // send crc of (cmd,TA,redir) at first, then only crc of (redir)
                crc=0;
                const uint16_t real_TA = translateRedirection(reg_TA);
                const uint8_t  length  = PAGE_SIZE - uint8_t(reg_TA & PAGE_MASK);
                if (hub->send(&memory[real_TA],length,crc))         break;
                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) break;
                reg_TA += length;
                crc=0;
            };
            break; // datasheed says we should return 1s, till reset, nothing to do here

        case 0xAA:      // READ STATUS
            while (reg_TA < sizeof_status) // check for valid address
            {
                reg_RA = reg_TA&uint8_t(7);
                //if (reg_RA >= STATUS_SIZE - 8) reg_RA = STATUS_SIZE - uint8_t(8) + (reg_RA & uint8_t(7)); // TODO: redirect
                const uint8_t  length  = uint8_t(8 - (reg_RA & 7));
                if (hub->send(&status[reg_RA],length,crc)) return;
                crc = ~crc; // normally crc16 is sent ~inverted
                hub->send(reinterpret_cast<uint8_t *>(&crc),2);
                reg_TA += length;
                crc = 0;
            };
            break;

        case 0x0F:      // WRITE MEMORY
            while (reg_TA < sizeof_memory) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) break;
                // master issues now a 480us 12V-Programming Pulse

                if (checkProtection(reg_TA)) break;
                reg_RA = translateRedirection(reg_TA);
                memory[reg_RA] &= data; // like EEPROM-Mode, TODO: mark page as used

                if (hub->send(&memory[reg_RA])) break;
                crc = ++reg_TA; // prepare new loop
            };
            break;

        case 0xF3:      // SPEED WRITE MEMORY, omit CRC
            while (reg_TA < sizeof_memory) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;
                // master issues now a 480us 12V-Programming Pulse

                if (checkProtection(reg_TA)) break;
                reg_RA = translateRedirection(reg_TA);
                memory[reg_RA] &= data; // like EEPROM-Mode, TODO: mark page as used

                if (hub->send(&memory[reg_RA])) break;
                ++reg_TA; // prepare new loop
            };
            break;

        case 0x55:      // WRITE STATUS
            while (reg_TA < sizeof_status) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) break;
                // master issues now a 480us 12V-Programming Pulse

                reg_RA = translateStatusAddress(reg_TA);
                status[reg_RA] &= data; // like EEPROM-Mode, TODO: mark page as used

                if (hub->send(&status[reg_RA])) break;
                crc = ++reg_TA; // prepare new loop
            };
            break;

        case 0xF5:      // SPEED WRITE STATUS, omit CRC
            while (reg_TA < sizeof_status) // check for valid address
            {
                if (hub->recv(&data,1,crc)) break;
                // master issues now a 480us 12V-Programming Pulse

                reg_RA = translateStatusAddress(reg_TA);
                status[reg_RA] &= data; // like EEPROM-Mode, TODO: mark page as used

                if (hub->send(&status[reg_RA])) break;
                ++reg_TA; // prepare new loop
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
};

void DS2506::clearMemory(void)
{
    memset(&memory[0], static_cast<uint8_t>(0xFF), MEM_SIZE);
};

void DS2506::clearStatus(void)
{
    memset(&status[0], static_cast<uint8_t>(0xFF), STATUS_SIZE);
    status[STATUS_SIZE-1] = 0x00; // last byte should be always zero, TODO: right for 2506?
};

bool DS2506::writeMemory(const uint8_t* source, const uint16_t length, const uint16_t position)
{
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);
    return (_length==length);
};

bool DS2506::readMemory(uint8_t* destination, const uint16_t length, const uint16_t position)
{
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
};

bool DS2506::checkProtection(const uint16_t reg_address)
{
    uint8_t reg_index = uint8_t(reg_address >> 5);
    return ((status[0] & (1<<reg_index)) == 0);
};

uint8_t DS2506::getRedirection(const uint16_t reg_address)
{
    if (reg_address >= MEM_SIZE) return uint8_t(~(reg_address & MEM_MASK)>>5); // out of bound is translated to last byte
    const uint16_t reg_index  = uint8_t(1) + (reg_address >> 5);
    return (status[reg_index]);
};

uint16_t DS2506::translateRedirection(const uint16_t reg_address) // TODO: extended read mem implies that redirection is recursive
{
    if (reg_address >= MEM_SIZE) return (reg_address & MEM_MASK); // out of bound is translated to last byte
    const uint16_t reg_index  = uint8_t(1) + (reg_address >> 5);
    const uint16_t reg_offset = (status[reg_index] == 0xFF) ? (reg_address) : ((~status[reg_index])<<5);
    return ((reg_offset & ~PAGE_MASK) | uint8_t(reg_address & PAGE_MASK));
};


uint16_t DS2506::translateStatusAddress(const uint16_t reg_address)
{
    return (0);
};

bool DS2506::redirectPage(const uint8_t page_source, const uint8_t page_dest)
{
    if (page_source >= PAGE_SIZE) return false;
    if (page_dest >= PAGE_SIZE) return false;

    status[page_source + 1] = (page_dest == page_source) ? uint8_t(0xFF) : ~page_dest; // datasheet dictates this, so no page can be redirected to page 0
    return true;
};

bool DS2506::protectPage(const uint8_t page, const bool status_protected)
{
    if (page > PAGE_SIZE) return false;

    status[0] &= ~(1<<page);
    status[0] |= ((!status_protected)&0x01)<<page;

    return true;
};
