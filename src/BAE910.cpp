#include "BAE910.h"

BAE910::BAE910(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 256,  "Implementation does not cover the whole address-space");
    static_assert(sizeof(sBAE910) <= BAE910_MEMORY_SIZE,  "Memory-struct is larger than its memory"); // not needed anymore, but not hurting either

    // clear memory
    memset(&memory.bytes[0], static_cast<uint8_t>(0x00), BAE910_MEMORY_SIZE);
}


void BAE910::duty(OneWireHub * const hub)
{
    uint8_t  cmd, ta1, ta2, len, eCmd; // command, targetAddress, length and extended command
    uint16_t crc { 0 };

    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0x11: // READ VERSION

            if (hub->send(&memory.field.SW_VER,1,crc))                return;
            if (hub->send(&memory.field.BOOTSTRAP_VER,1,crc))         return;

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break;

        case 0x12: // READ TYPE

            if (hub->send(&BAE910_DEVICE_TYPE,1,crc))           return;
            if (hub->send(&BAE910_CHIP_TYPE,1,crc))             return;

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break;

        case 0x14: // READ MEMORY

            if (hub->recv(&ta1,1,crc))                          return;
            if (hub->recv(&ta2,1,crc))                          return;
            if (hub->recv(&len,1,crc))                          return;

            if ((ta1 + len > 0x80) || (ta2 > 0))
            {
                hub->raiseSlaveError(cmd);
                return;
            }
            // reverse byte order
            for (uint8_t i = 0; i < len; ++i)
            {
                if (hub->send(&memory.bytes[0x7F - ta1 - i],1,crc))  return;
            }

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break;

        case 0x15: // WRITE MEMORY

            if (hub->recv(&ta1,1,crc))                          return;
            if (hub->recv(&ta2,1,crc))                          return;
            if (hub->recv(&len,1,crc))                          return;

            if ((len > BAE910_SCRATCHPAD_SIZE) || (ta1 + len > 0x80) || (ta2 > 0))
            {
                hub->raiseSlaveError(cmd);
                return;
            }

            if (hub->recv(scratchpad,len,crc))                  return;

            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            // verify answer from master, then copy memory
            if (hub->recv(&eCmd ,1))                             return;
            if (eCmd == 0xBC)
            {
                while (len-- > 0) // reverse byte order
                {
                    memory.bytes[0x7F - ta1 - len] = scratchpad[len];
                }
            }
            break;

//        case 0x13: // EXTENDED COMMAND
//        case 0x16: // ERASE EEPROM PAGE (not needed/implemented yet)
        default:

            hub->raiseSlaveError(cmd);
    }
}
