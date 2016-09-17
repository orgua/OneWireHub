#include "DS2431.h"

DS2431::DS2431(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    checkMemory();
};

bool DS2431::duty(OneWireHub *hub)
{
    static uint16_t register_ta = 0; // contains TA1, TA2
    static uint8_t  register_es = 0;  // E/S register
    static uint16_t crc = 0;

    uint8_t  mem_counter = 0; // offer feedback with PF-bit

    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;
    uint8_t  b;
    switch (cmd)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            crc = crc16(0x0F, 0x00);//CRC_INIT);
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[0] = b;
            register_es = b & uint8_t(0x00000111); // TODO: when not zero we should issue register_es |= 0b00100000;
            crc = crc16(b, crc);

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[1] = b;
            crc = crc16(b, crc);

            // up to 8 bytes of data
            for (uint8_t i = register_es; i < 8; ++i) { // address can point directly to offset-byte in scratchpad
                b = hub->recv();
                if (hub->getError())
                {
                    // set the PF-Flag if error occured in the middle of the byte
                    if (hub->clearError() != Error::FIRST_BIT_OF_BYTE_TIMEOUT) register_es |= 0b00100000;
                    break;
                }
                if (register_es < 7) register_es++;
                scratchpad[i] = b;
                crc = crc16(b, crc);
            };

            // check if page is protected or in eprom-mode
            if (checkProtection(register_ta))
            {
                // protected: load memory-segment to scratchpad
                const uint8_t position = register_ta & 0b11111000;
                for (uint8_t i = 0; i < 8; ++i)
                {
                    scratchpad[i] = memory[position + i];
                }
            }
            else if (checkEpromMode(register_ta))
            {
                // eprom: logical AND of memory and datav
                const uint8_t position = register_ta & 0b11111000;
                for (uint8_t i = 0; i < 8; ++i)
                {
                    scratchpad[i] &= memory[position + i];
                }
            };

            if (register_es & 0b00100000) return false; // only partial filling of bytes

            // CRC-16
            crc = ~crc; // normally crc16 is sent ~inverted
            hub->send(uint8_t(reinterpret_cast<uint8_t *>(&crc)[0]));
            if (hub->getError())  return false;
            hub->send(uint8_t(reinterpret_cast<uint8_t *>(&crc)[1]));

            break;

        // READ SCRATCHPAD COMMAND
        case 0xAA:
            crc = crc16(0xAA, 0);
            // Write-to address
            hub->send(reinterpret_cast<uint8_t *>(&register_ta)[0]);
            crc = crc16(reinterpret_cast<uint8_t *>(&register_ta)[0], crc);
            if (hub->getError())  return false;

            hub->send(reinterpret_cast<uint8_t *>(&register_ta)[1]);
            crc = crc16(reinterpret_cast<uint8_t *>(&register_ta)[1], crc);
            if (hub->getError())  return false;

            hub->send(register_es);
            crc = crc16(register_es, crc);
            if (hub->getError())  return false;

            //Scratchpad content
            for (uint8_t i = (register_ta & 0x03); i < (register_es & 0x03)+1; ++i) {
                hub->send(scratchpad[i]);
                if (hub->getError()) return false; // master can break, but gets no crc afterwards
                crc = crc16(scratchpad[i], crc);
            };
            
            // CRC-16
            crc = ~crc;
            hub->send(uint8_t(reinterpret_cast<uint8_t *>(&crc)[0]));
            if (hub->getError())  return false;
            hub->send(uint8_t(reinterpret_cast<uint8_t *>(&crc)[1]));
            if (hub->getError())  return false;

            while (1) // send 1s when read is complete
            {
                hub->send(255);
                if (hub->getError()) return false;
            };

            break;
        
        // COPY SCRATCHPAD COMMAND
        case 0x55:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            if (b != reinterpret_cast<uint8_t *>(&register_ta)[0]) break;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            if (b != reinterpret_cast<uint8_t *>(&register_ta)[1]) break; // Write-to addresses must match
            
            // Auth code must match
            b = hub->recv();
            if (hub->getError())  return false;
            if (b != register_es) break;

            if (register_es & 0b00100000) break; // writing failed before

            register_ta &= ~uint16_t(0b00000111);

            // Write Scratchpad
            writeMemory(scratchpad, 8, register_ta); // checks if copy protected

            // set the auth code uppermost bit, AA
            register_es |= 0b10000000;
            delayMicroseconds(10000); // writing takes so long
            hub->extendTimeslot();
            hub->sendBit(1);
            hub->clearError();
            hub->extendTimeslot();
            while (1) // send 1s when alternating 1 & 0 after copy is complete
            {
                hub->send(0b10101010);
                if (hub->getError())
                {
                    break;
                };
            };

            break;
        
        // READ MEMORY COMMAND
        case 0xF0:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[1] = b;
 
            for (uint16_t index_byte = register_ta; index_byte < sizeof(memory); ++index_byte)
            {
                hub->send(memory[index_byte]);
                if (hub->getError())  break;
            }

            if (hub->getError())  break;

            while (1) // send 1s when read is complete
            {
                hub->send(255);
                if (hub->getError())
                {
                    break;
                };
            };

            break;

        default:
            hub->raiseSlaveError(cmd);

    };
    return !(hub->getError());
};

bool DS2431::checkProtection(const uint8_t position)
{
    if      (position <  32)
    {
        if (page_protection & 1) return true; // not pretty
    }
    else if (position <  64)
    {
        if (page_protection & 2) return true;
    }
    else if (position <  96)
    {
        if (page_protection & 4) return true;
    }
    else if (position < 128)
    {
        if (page_protection & 8) return true;
    }
    else if (position > 127)
    {
        if (page_protection & 16) return true;
    };
    return false;
};

bool DS2431::checkEpromMode(const uint8_t position)
{
    if      (position <  32)
    {
        if (page_eprom_mode & 1) return true;
    }
    else if (position <  64)
    {
        if (page_eprom_mode & 2) return true;
    }
    else if (position <  96)
    {
        if (page_eprom_mode & 4) return true;
    }
    else if (position < 128)
    {
        if (page_eprom_mode & 8) return true;
    }
    else if (position > 127)
    {
        if (page_eprom_mode & 16) return true;
    };
    return false;
};


bool DS2431::clearMemory(void)
{
    for (int i = 0; i < sizeof(memory); ++i)
    {
        memory[i] = 0x00;
    };
    return true;
};

bool DS2431::writeMemory(const uint8_t* source, const uint8_t length, const uint8_t position)
{
    if (checkProtection(position)) return false;

    for (uint8_t i = 0; i < length; ++i) {
        if ((position + i) >= sizeof(memory)) break;
        memory[position + i] = source[i];
    };

    if (position & 0x80) checkMemory();

    return true;
};

bool DS2431::checkMemory(void)
{
    constexpr uint8_t WPM = 0x55; // write protect mode
    constexpr uint8_t EPM = 0xAA; // eprom mode

    page_eprom_mode = 0;
    page_protection = 0;

    if (memory[0x80] == WPM) page_protection |= 1;
    if (memory[0x81] == WPM) page_protection |= 2;
    if (memory[0x82] == WPM) page_protection |= 4;
    if (memory[0x83] == WPM) page_protection |= 8;
    if (memory[0x83] == WPM) page_protection |= 8;
    if (memory[0x84] == WPM) page_protection |= 16;
    if (memory[0x84] == EPM) page_protection |= 16;

    if (memory[0x80] == EPM) page_eprom_mode |= 1;
    if (memory[0x81] == EPM) page_eprom_mode |= 2;
    if (memory[0x82] == EPM) page_eprom_mode |= 4;
    if (memory[0x83] == EPM) page_eprom_mode |= 8;
    if (memory[0x83] == EPM) page_eprom_mode |= 8;
    return true;
};