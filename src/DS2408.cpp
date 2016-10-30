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

void DS2408::duty(OneWireHub *hub)
{
    constexpr uint8_t DATA_xAA = 0xAA;
    uint8_t cmd, ta1, data; // command, targetAdress and databytes
    uint16_t crc = 0, crc2;
    if (hub->recv(&cmd,1,crc)) return;

    switch (cmd)
    {
        case 0xF0:      // Read PIO Registers
            if (hub->recv(&ta1,1,crc)) return;
            if((ta1 < DS2408_OFFSET) || (ta1 >= DS2408_OFFSET + DS2408_MEMSIZE)) return;
            if (hub->recv(&data,1,crc)) return;
            if (data != 0) return;

            {
                const uint8_t start  = (ta1 - DS2408_OFFSET);
                const uint8_t length = DS2408_MEMSIZE - start;
                if (hub->send(&memory[start],length,crc)) return;
            }

            crc = ~crc; // most important step, easy to miss....
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break;
            // after memory readout this chip sends logic 1s, which is the same as staying passive

        case 0x5A:      // Channel-Access Write
            while(1)
            {
                if (hub->recv(&data,1,crc)) return;
                if (hub->recv(&cmd ,1,crc)) return; // just because we have to receive somethin
                //if (cmd != ~data) return false; //inverted data, not working properly

                memory[DS2408_PIO_ACTIVITY_REG] |= data ^ memory[DS2408_PIO_LOGIC_REG];
                memory[DS2408_PIO_OUTPUT_REG]   = data;
                memory[DS2408_PIO_LOGIC_REG]    = data;
                if (hub->send(&DATA_xAA)) return;
                if (hub->send(memory,4)) return; // TODO: i think this is right, datasheet says: DS2408 samples the status of the PIO pins, as shown in Figure 9, and sends it to the master
            }

        case 0xF5:      // Channel-Access Read
            crc2 = crc;
            while (1)
            {
                crc = crc2;
                if (hub->send(memory,4,crc)) return;
                crc = ~crc; // most important step, easy to miss....
                if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            };

        case 0xC3:      // reset activity latches
            memory[DS2408_PIO_ACTIVITY_REG] = 0x00;
            while(1)
            {
                if (hub->send(&DATA_xAA)) return;
            };

        case 0xCC:      // write conditional search register
            // TODO: page 18 datasheet
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
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
