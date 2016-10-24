#include "DS2408.h"

DS2408::DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    memory[DS2408_PIO_LOGIC_REG]            = 0xFF;
    memory[DS2408_PIO_OUTPUT_REG]           = 0xFF;
    memory[DS2408_PIO_ACTIVITY_REG]         = 0xFF;
    memory[DS2408_SEARCH_MASK_REG]          = 0;
    memory[DS2408_SEARCH_SELECT_REG]        = 0;
    memory[DS2408_CONTROL_STATUS_REG]       = 0x88;
    memory[DS2408_RD_ABOVE_ALWAYS_FF_8E]    = 0xFF;
    memory[DS2408_RD_ABOVE_ALWAYS_FF_8F]    = 0xFF;
};

bool DS2408::duty(OneWireHub *hub)
{
    uint8_t targetAddress;
    uint16_t crc = 0, crc2;
    uint8_t cmd = hub->recvAndCRC16(crc);
    uint8_t data;
    if (hub->getError())  return false;

    switch (cmd)
    {
        case 0xF0:      // Read PIO Registers
            targetAddress = hub->recvAndCRC16(crc);
            if (hub->getError())  return false;
            if((targetAddress < DS2408_OFFSET) || (targetAddress >= DS2408_OFFSET + DS2408_MEMSIZE)) return false;
            if (hub->recvAndCRC16(crc) != 0) return false;
            if (hub->getError())  return false;

            for (uint8_t count = (targetAddress - DS2408_OFFSET); count < DS2408_MEMSIZE; ++count)
            {
                crc = hub->sendAndCRC16(memory[count], crc);
                if (hub->getError()) return false;
            }
            crc = ~crc; // most important step, easy to miss....
            hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
            if (hub->getError())  return false;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
            if (hub->getError())  return false;
            // after memory readout this chip sends logic 1s, which is the same as staying passive
            break;

        case 0x5A:      // Channel-Access Write
            while(1)
            {
                data = hub->recv();
                if (hub->getError()) return false;
                //if (hub->recv() != ~data) return false; //inverted data, not working properly
                hub->recv();
                if (hub->getError()) return false;
                memory[DS2408_PIO_ACTIVITY_REG] |= data ^ memory[DS2408_PIO_LOGIC_REG];
                memory[DS2408_PIO_OUTPUT_REG]   = data;
                memory[DS2408_PIO_LOGIC_REG]    = data;
                hub->send(0xAA);
                if (hub->getError()) return false;
                for (uint8_t count = 0; count < 4; ++count) // TODO: i think this is right, datasheet says: DS2408 samples the status of the PIO pins, as shown in Figure 9, and sends it to the master
                {
                    hub->send(memory[count]);
                    if (hub->getError()) return false;
                }
            }

        case 0xF5:      // Channel-Access Read
            crc2 = crc;
            while (1)
            {
                crc = crc2;
                for (uint8_t count = 0; count < 4; ++count)
                {
                    crc = hub->sendAndCRC16(memory[count], crc);
                    if (hub->getError()) return false;
                }
                crc = ~crc; // most important step, easy to miss....
                hub->send(reinterpret_cast<uint8_t *>(&crc)[0]);
                if (hub->getError()) return false;
                hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);
                if (hub->getError()) return false;
            };

        case 0xC3:      // reset activity latches
            memory[DS2408_PIO_ACTIVITY_REG] = 0x00;
            while(1)
            {
                hub->send(0xAA);
                if (hub->getError()) return false;
            };

        case 0xCC:      // write conditional search register
            // TODO: page 18 datasheet
            break;

        default:
            hub->raiseSlaveError(cmd);
    };

    return !(hub->getError());
};

bool DS2408::getPinState(uint8_t pinNumber)
{
    return static_cast<bool>(memory[DS2408_PIO_LOGIC_REG] & ( 1 << pinNumber ));
};

uint8_t DS2408::getPinStates(void)
{
    return memory[DS2408_PIO_LOGIC_REG];
};

void DS2408::setPinState(uint8_t pinNumber, bool value)
{
    uint8_t pio_state = memory[DS2408_PIO_LOGIC_REG];
    if(value)   pio_state |= 1 << pinNumber;
    else        pio_state &= ~(1 << pinNumber);

    // look for changes in the activity latches
    memory[DS2408_PIO_ACTIVITY_REG] |= pio_state ^ memory[DS2408_PIO_LOGIC_REG];
    memory[DS2408_PIO_LOGIC_REG]    = pio_state;
    memory[DS2408_PIO_OUTPUT_REG]   = pio_state;
};
