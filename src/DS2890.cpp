#include "OneWireHub.h"
#include "DS2890.h"

DS2890::DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    register_feat = REGISTER_MASK_POTI_CHAR | REGISTER_MASK_WIPER_SET | REGISTER_MASK_WIPER_POS | REGISTER_MASK_POTI_RESI;
    register_poti[0] = 0;
    register_poti[1] = 0;
    register_poti[2] = 0;
    register_poti[3] = 0;
    register_ctrl    = 0b00001100;
}

bool DS2890::duty(OneWireHub *hub)
{
    uint8_t temp = 0;
    uint8_t done = hub->recv();

    switch (done)
    {

        case 0x0F: // WRITE POSITION
            temp = hub->recv();
            hub->send(temp);
            done = hub->recv();
            if (done == 0x96) // release code received
                register_poti[register_ctrl&0x03] = temp;

            if (dbg_sensor)
            {
                Serial.print("DS2890 : WRITE POSITION: ");
                Serial.println(register_poti[register_ctrl&0x03], HEX);
            }
            break;


        case 0x55: // WRITE CONTROL REGISTER
            temp = hub->recv();

            if (temp&0x01) temp |= 0x04;
            else temp &= ~0x04;
            if (temp&0x02) temp |= 0x08;
            else temp &= ~0x08;

            hub->send(temp);

            done = hub->recv();
            if (done == 0x96) // release code received
                register_ctrl = temp;

            if (dbg_sensor)
            {
                Serial.print("DS2890 : WRITE CONTROL REGISTER: ");
                Serial.println(temp, HEX);
            }
            break;


        case 0xAA: // READ CONTROL REGISTER
            hub->send(register_ctrl);
            hub->send(register_feat);

            if (dbg_sensor)
            {
                Serial.print("DS2890 : READ CONTROL REGISTER: ");
                Serial.print(register_ctrl, HEX);
                Serial.print("-");
                Serial.println(register_feat, HEX);
            }
            break;


        case 0xF0: // READ POSITION
            hub->send(register_ctrl);
            hub->send(register_poti[register_ctrl&0x03]);

            if (dbg_sensor)
            {
                Serial.print("DS2890 : READ POSITION: ");
                Serial.print(register_ctrl, HEX);
                Serial.print("-");
                Serial.println(register_poti[register_ctrl&0x03], HEX);
            }
            break;


        case 0xC3: // INCREMENT
            if (register_poti[register_ctrl&0x03] < 0xFF) register_poti[register_ctrl&0x03]++;
            hub->send(register_poti[register_ctrl&0x03]);
            if (dbg_sensor) Serial.print("DS2890 : INCREMENT");
            break;


        case 0x99: // DECREMENT
            if (register_poti[register_ctrl&0x03]) register_poti[register_ctrl&0x03]--;
            hub->send(register_poti[register_ctrl&0x03]);
            if (dbg_sensor) Serial.print("DS2890 : DECREMENT");
            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2890=");
                Serial.println(done, HEX);
            }
            break;
    }

    return true;
}
