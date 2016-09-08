#include "DS2890.h"

DS2890::DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    register_feat = REGISTER_MASK_POTI_CHAR | REGISTER_MASK_WIPER_SET | REGISTER_MASK_WIPER_POS | REGISTER_MASK_POTI_RESI;
    register_poti[0] = 0;
    register_poti[1] = 0;
    register_poti[2] = 0;
    register_poti[3] = 0;
    register_ctrl    = 0b00001100;
};

bool DS2890::duty(OneWireHub *hub)
{
    uint8_t temp = 0;
    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;

    switch (cmd)
    {

        case 0x0F: // WRITE POSITION
            temp = hub->recv();
            if (hub->getError())  return false;
            hub->send(temp);
            if (hub->getError())  return false;
            cmd = hub->recv();
            if (hub->getError())  return false;

            // release code received
            if (cmd == 0x96)      register_poti[register_ctrl&0x03] = temp;

            break;


        case 0x55: // WRITE CONTROL REGISTER
            temp = hub->recv();
            if (hub->getError())  return false;

            if (temp&0x01) temp |= 0x04;
            else temp &= ~0x04;
            if (temp&0x02) temp |= 0x08;
            else temp &= ~0x08;

            hub->send(temp);
            if (hub->getError())  return false;

            cmd = hub->recv();
            if (hub->getError())  return false;

            // release code received
            if (cmd == 0x96)      register_ctrl = temp;

            break;


        case 0xAA: // READ CONTROL REGISTER
            hub->send(register_ctrl);
            if (hub->getError())  return false;
            hub->send(register_feat);
            break;

        case 0xF0: // READ POSITION
            hub->send(register_ctrl);
            if (hub->getError())  return false;
            hub->send(register_poti[register_ctrl&0x03]);
            break;

        case 0xC3: // INCREMENT
            if (register_poti[register_ctrl&0x03] < 0xFF) register_poti[register_ctrl&0x03]++;
            hub->send(register_poti[register_ctrl&0x03]);
            break;

        case 0x99: // DECREMENT
            if (register_poti[register_ctrl&0x03]) register_poti[register_ctrl&0x03]--;
            hub->send(register_poti[register_ctrl&0x03]);
            break;

        default:
            hub->raiseSlaveError(cmd);
    };

    return !(hub->getError());
};
