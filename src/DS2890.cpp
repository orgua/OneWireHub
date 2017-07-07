#include "DS2890.h"

DS2890::DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    register_feat = REG_MASK_POTI_CHAR | REG_MASK_WIPER_SET | REG_MASK_POTI_NUMB | REG_MASK_WIPER_POS | REG_MASK_POTI_RESI;
    memset(register_poti, uint8_t(0), 4);
    register_ctrl    = 0b00001100;
}

void DS2890::duty(OneWireHub * const hub)
{
    const uint8_t poti = register_ctrl&POTI_MASK;
    uint8_t data, cmd;

    start_over:

    if (hub->recv(&cmd))  return;

    switch (cmd)
    {
        case 0x0F:      // WRITE POSITION

            if (hub->recv(&data))           break;
            if (hub->send(&data))           break;
            if (hub->recv(&cmd))            break;

            if (cmd == RELEASE_CODE) register_poti[poti] = data;
            break; // respond with 1s ... passive

        case 0x55:      // WRITE CONTROL REGISTER

            if (hub->recv(&data))           break;
            if (hub->send(&data))           break;
            if (hub->recv(&cmd))            break;

            if (cmd == RELEASE_CODE)
            {
                if ((data&0x01) != 0) data &= ~0x04;
                else                  data |= 0x04;
                if ((data&0x02) != 0) data &= ~0x08;
                else                  data |= 0x08;

                register_ctrl = data;
            }
            break; // respond with 1s ... passive

        case 0xAA:      // READ CONTROL REGISTER

            if (hub->send(&register_feat))  break;
            if (hub->send(&register_ctrl))  break;
            noInterrupts();
            while (!hub->sendBit(false));
            interrupts();
            break;

        case 0xF0:      // READ POSITION

            if (hub->send(&register_ctrl))  break;
            if (hub->send(&register_poti[poti])) break;
            noInterrupts();
            while (!hub->sendBit(false));
            interrupts();
            break;

        case 0xC3:      // INCREMENT

            if (register_poti[poti] < 0xFF) register_poti[poti]++;
            if (hub->send(&register_poti[poti])) break;
            break;

        case 0x99:      // DECREMENT

            if (register_poti[poti] != 0) register_poti[poti]--;
            if (hub->send(&register_poti[poti])) break;
            break;

        default:

            hub->raiseSlaveError(cmd);
    }

    if ((cmd == 0xC3) || (cmd == 0x99)) goto start_over; // only for this device -> when INCREMENT or DECREMENT the master can issue another cmd right away
}
