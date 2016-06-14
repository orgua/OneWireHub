#include "BAE910.h"

BAE910::BAE910(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (uint8_t i = 0; i < 0x80; ++i)
        memory.bytes[i] = 0x00;
}

void BAE910::extCommand(uint8_t ecmd, uint8_t payload_len)
{
    // reserved:
    // 0xBB  Erase Firmware
    // 0xBA  Flash Firmware
}

bool BAE910::duty(OneWireHub *hub)
{
    uint8_t ta1;
    uint8_t ta2;
    uint8_t len;
    uint8_t ecmd;
    uint16_t crc = 0;

    uint8_t cmd  = hub->recvAndCRC16(crc);

    switch (cmd)
    {
        case 0x11: // READ VERSION
            crc = hub->sendAndCRC16(BAE910_SW_VER,        crc);
            crc = hub->sendAndCRC16(BAE910_BOOTSTRAP_VER, crc);
            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            break;

        case 0x12: // READ TYPE
            crc = hub->sendAndCRC16(BAE910_DEVICE_TYPE,   crc);
            crc = hub->sendAndCRC16(BAE910_CHIP_TYPE,     crc);
            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            break;

        case 0x13: // EXTENDED COMMAND
            ecmd = hub->recvAndCRC16(crc);
            len  = hub->recvAndCRC16(crc);
            if (len > BAE910_SCRATCHPAD_SIZE) {
                hub->raiseSlaveError(cmd);
                return false;
            }
            for( uint8_t i = 0; i < len; ++i )
                scratchpad[i] = hub->recvAndCRC16(crc);
            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            // verify answer from master, then execute command
            if (hub->recv() == 0xBC)
                extCommand(ecmd, len);
            break;

        case 0x14: // READ MEMORY
            ta1 = hub->recvAndCRC16(crc);
            ta2 = hub->recvAndCRC16(crc);
            len = hub->recvAndCRC16(crc);
            if ((ta1 + len > 0x80) || (ta2 > 0)) {
                hub->raiseSlaveError(cmd);
                return false;
            }
            // reverse byte order
            for (uint8_t i = 0; i < len; ++i)
                crc = hub->sendAndCRC16(memory.bytes[0x7F - ta1 - i], crc);
            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            break;

        case 0x15: // WRITE MEMORY
            ta1 = hub->recvAndCRC16(crc);
            ta2 = hub->recvAndCRC16(crc);
            len = hub->recvAndCRC16(crc);
            if ((len > BAE910_SCRATCHPAD_SIZE) || (ta1 + len > 0x80) || (ta2 > 0)) {
                hub->raiseSlaveError(cmd);
                return false;
            }
            for (uint8_t i = 0; i < len; ++i)
                scratchpad[i] = hub->recvAndCRC16(crc);
            // crc
            crc = ~crc;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            // verify answer from master, then copy memory
            if (hub->recv() == 0xBC)
                // reverse byte order
                while ( len-- > 0 )
                    memory.bytes[0x7F - ta1 - len] = scratchpad[len];
            break;

        case 0x16: // ERASE EEPROM PAGE (not needed/implemented yet)
        default:
            hub->raiseSlaveError(cmd);
            return false;
    }

    return true;
}
