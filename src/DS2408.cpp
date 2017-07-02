#include "DS2408.h"

DS2408::DS2408(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    clearMemory();
}

void DS2408::duty(OneWireHub * const hub)
{
    constexpr uint8_t DATA_xAA { 0xAA };
    uint8_t cmd, reg_TA, data; // command, targetAdress and databytes
    uint16_t crc { 0 };

    if (hub->recv(&cmd,1,crc)) return;

    switch (cmd)
    {
        case 0xF0:      // Read PIO Registers

            if (hub->recv(&reg_TA,1,crc)) return;
            if((reg_TA < REG_OFFSET) || (reg_TA >= REG_OFFSET + MEM_SIZE)) return;
            if (hub->recv(&data,1,crc)) return; // seconds part of reg_TA, should be zero
            if (data != 0) return;

            {
                const uint8_t start  = (reg_TA - REG_OFFSET);
                const uint8_t length = MEM_SIZE - start;
                if (hub->send(&memory[start],length,crc)) return;
            }

            crc = ~crc; // most important step, easy to miss....
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            break; // after memory readout this chip sends logic 1s, which is the same as staying passive

        case 0x5A:      // Channel-Access Write

            while(true)
            {
                if (hub->recv(&data,1)) return;
                if (hub->recv(&cmd ,1)) return; // just because we have to receive something
                //if (cmd != ~data) return false; //inverted data, not working properly

                memory[REG_PIO_ACTIVITY] |= data ^ memory[REG_PIO_LOGIC];
                memory[REG_PIO_OUTPUT]   = data;
                memory[REG_PIO_LOGIC]    = data;
                if (hub->send(&DATA_xAA)) return;
                if (hub->send(memory,4)) return; // TODO: i think this is right, datasheet says: DS2408 samples the status of the PIO pins, as shown in Figure 9, and sends it to the master
            }

        case 0xF5:      // Channel-Access Read

            while (true)
            {
                static uint16_t crc2 = crc;
                if (hub->send(memory,4,crc)) return;
                crc = ~crc; // most important step, easy to miss....
                if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
                crc = crc2;
            }

        case 0xC3:      // reset activity latches

            memory[REG_PIO_ACTIVITY] = 0x00;
            while(!hub->send(&DATA_xAA));

        case 0xCC:      // write conditional search register

            if (hub->recv(&reg_TA,1))                   return;
            if(reg_TA < (REG_SEARCH_MASK + REG_OFFSET)) return;
            if (hub->recv(&data,1))                     return; // seconds part of reg_TA, should be zero
            if (data != 0)                              return;

            while(reg_TA <= REG_CONTROL_STATUS + REG_OFFSET)
            {
                if (hub->recv(&memory[reg_TA - REG_OFFSET],1)) return;
            }
            // TODO: page 18 datasheet, no alarm search yet, control-register has influence
            break;

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2408::clearMemory(void)
{
    memory[REG_PIO_LOGIC]               = 0xFF;
    memory[REG_PIO_OUTPUT]              = 0xFF;
    memory[REG_PIO_ACTIVITY]            = 0xFF;
    memory[REG_SEARCH_MASK]             = 0;
    memory[REG_SEARCH_SELECT]           = 0;
    memory[REG_CONTROL_STATUS]          = 0x88;
    memory[REG_RD_ABOVE_ALWAYS_FF_8E]   = 0xFF;
    memory[REG_RD_ABOVE_ALWAYS_FF_8F]   = 0xFF;
}

void DS2408::setPinState(const uint8_t pinNumber, const bool value)
{
    uint8_t pio_state = memory[REG_PIO_LOGIC];
    if(value)   pio_state |= 1 << pinNumber;
    else        pio_state &= ~(1 << pinNumber);

    // look for changes in the activity latches
    memory[REG_PIO_ACTIVITY] |= pio_state ^ memory[REG_PIO_LOGIC]; // TODO: just good guess here, has anyone the energy to figure out each register?
    memory[REG_PIO_LOGIC]    = pio_state;
    memory[REG_PIO_OUTPUT]   = pio_state;
}

bool DS2408::getPinState(const uint8_t pinNumber) const
{
    return static_cast<bool>(memory[REG_PIO_LOGIC] & ( 1 << pinNumber ));
}

uint8_t DS2408::getPinState(void) const
{
    return memory[REG_PIO_LOGIC];
}

void DS2408::setPinActivity(const uint8_t pinNumber, const bool value)
{
    if (value)  memory[REG_PIO_ACTIVITY] |=  (1<<pinNumber);
    else        memory[REG_PIO_ACTIVITY] &= ~(1<<pinNumber);
}

bool DS2408::getPinActivity(const uint8_t pinNumber) const
{
    return static_cast<bool>(memory[REG_PIO_ACTIVITY] & ( 1 << pinNumber ));
}

uint8_t DS2408::getPinActivity(void) const
{
    return memory[REG_PIO_ACTIVITY];
}
