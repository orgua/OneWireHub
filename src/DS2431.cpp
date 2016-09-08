#include "DS2431.h"

DS2431::DS2431(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
};

bool DS2431::duty(OneWireHub *hub)
{
    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;
    uint8_t  b;
    switch (cmd)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_w)[0] = b;
            crcArg[1] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_w)[1] = b;
            crcArg[2] = b;

            // 8 bytes of data
            for (uint8_t i = 0; i < 8; ++i) {
                b = hub->recv();
                if (hub->getError())
                {
                    hub->clearError();
                    break;
                }
                scratchpad[i] = b;
                crcArg[3 + i] = b;
            };

            // Compute the auth code
            auth_code = (memory_address_w >> 8) + 7;

            // Compute the CRC
            crc = crc16(crcArg, 11, CRC_INIT);
            break;

        // READ SCRATCHPAD COMMAND
        case 0xAA:
            // Write-to address
            hub->send(memory_address_w & 0xff);
            if (hub->getError())  return false;
            hub->send(memory_address_w >> 8);
            if (hub->getError())  return false;
            
            // Auth code
            hub->send(auth_code);
            if (hub->getError())  return false;
            
            //Scratchpad content
            for (uint8_t i = 0; i < 8; ++i) { // TODO: address can point directly to offset-byte in scratchpad, so this code is not fully compliant (same above)
                hub->send(scratchpad[i]);
                if (hub->getError()) return false; // master can break, but gets no crc afterwards
            };
            
            // CRC-16
            hub->send(crc & 0xff);
            if (hub->getError())  return false;
            hub->send((crc >> 8));
            break;
        
        // COPY SCRATCHPAD COMMAND
        case 0x55:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_w_confirmation)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_w_confirmation)[1] = b;
            
            // Write-to addresses must match
            if (memory_address_w != memory_address_w_confirmation) break;
            
            // Auth code must match
            b = hub->recv();
            if (b != auth_code)   break;
            
            // Write Scratchpad
            for (uint8_t i = 0; i < 8; ++i) {
                memory[memory_address_w + i] = scratchpad[i];
            }

            // Change the auth code uppermost bit
            auth_code = auth_code ^ 0x80;
            break;
        
        // READ MEMORY COMMAND
        case 0xF0:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_r)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&memory_address_r)[1] = b;
 
            for (; memory_address_r < sizeof(memory); ++memory_address_r)
            {
                hub->send(memory[memory_address_r]);
                if (hub->getError())  break;
            }
            break;

        default:
            hub->raiseSlaveError(cmd);

    };
    return !(hub->getError());
};
