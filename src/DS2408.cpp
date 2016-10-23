#include "DS2408.h"

DS2408::DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    memory.registers[DS2408_PIO_LOGIC_REG] = 0x41;
    memory.registers[DS2408_PIO_OUTPUT_REG] = 0xFF;
    memory.registers[DS2408_PIO_ACTIVITY_REG] = 0xFF;
    memory.registers[DS2408_SEARCH_MASK_REG] = 0;
    memory.registers[DS2408_SEARCH_SELECT_REG] = 0;
    memory.registers[DS2408_CONTROL_STATUS_REG] = 0x88;
    memory.registers[DS2408_RD_ABOVE_ALWAYS_FF_8E] = 0xFF;
    memory.registers[DS2408_RD_ABOVE_ALWAYS_FF_8F] = 0xFF;
};

bool DS2408::duty(OneWireHub *hub)
{
    uint8_t targetAddress;
    uint16_t crc = 0;
    uint8_t cmd = hub->recvAndCRC16(crc);
    uint8_t data;
    if (hub->getError())  return false;

    switch (cmd)
    {
        case 0xF0:      // Read PIO Registers
            targetAddress = hub->recvAndCRC16(crc);
            if (hub->getError())  return false;
            if(targetAddress < 0x88 || targetAddress > 0x8F) return false;
            hub->recvAndCRC16(crc);
            if (hub->getError())  return false;

            for (uint8_t count = targetAddress - 0x88; count < 8; ++count)
            {
                crc = hub->sendAndCRC16(memory.registers[count], crc);
                if (hub->getError()) return false;
            }
            crc = ~crc; // most important step, easy to miss....
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            if (hub->getError())  return false;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            break;

        case 0x5A:      // Channel-Access Write
            data = hub->recv();
            if (hub->getError())  return false;
            hub->recv(); //inverted data
            if (hub->getError())  return false;
            memory.registers[DS2408_PIO_OUTPUT_REG] = data;
            memory.registers[DS2408_PIO_LOGIC_REG] = memory.registers[DS2408_PIO_OUTPUT_REG];
            hub->send(0xAA);
            if (hub->getError())  return false;
            hub->send(memory.registers[DS2408_PIO_OUTPUT_REG]);
            break;

        default:
            hub->raiseSlaveError(cmd);
    };

    return !(hub->getError());
};

bool DS2408::getPinState(uint8_t pinNumber)
{
    return memory.registers[DS2408_PIO_LOGIC_REG] & 1 << pinNumber;
};

void DS2408::setPinState(uint8_t pinNumber, bool value)
{
    if(value)
        memory.registers[DS2408_PIO_LOGIC_REG] |= 1 << pinNumber;
    else
        memory.registers[DS2408_PIO_LOGIC_REG] &= ~(1 << pinNumber);
};
