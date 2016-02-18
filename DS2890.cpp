#include "OneWireHub.h"
#include "DS2890.h"

const bool dbg_DS2890 = 0; // give debug messages for this sensor

DS2890::DS2890(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    regs = 0;
    potion = 0;
}

bool DS2890::duty(OneWireHub *hub)
{
    uint16_t data; // TODO: unused for now

    uint8_t done = hub->recv();
    switch (done)
    {
        // WRITE POSITION
        case 0x0F:
            // get
            potion = hub->recv();

            // send
            hub->send(potion);

        if (dbg_DS2890)
        {
            Serial.print("DS2890 : WRITE POSITION: ");
            Serial.println(potion, HEX);
        }

            break;

            // WRITE CONTROL REGISTER
        case 0x55:
            // data
            regs = hub->recv();

            // send dara
            hub->send(regs);

            if (dbg_DS2890)
            {
                Serial.print("DS2890 : WRITE CONTROL REGISTER: ");
                Serial.println(regs, HEX);
            }

            break;

            // READ CONTROL REGISTER
        case 0xAA:
            // regs
            hub->send(regs);

            // send
            hub->send(potion);

            if (dbg_DS2890)
            {
                Serial.print("DS2890 : READ CONTROL REGISTER: ");
                Serial.print(regs, HEX);
                Serial.print("-");
                Serial.println(potion, HEX);
            }

            break;

            // READ POSITION
        case 0xF0:
            // regs
            hub->send(regs);

            // send
            hub->send(potion);

            if (dbg_DS2890)
            {
                Serial.print("DS2890 : READ POSITION: ");
                Serial.print(regs, HEX);
                Serial.print("-");
                Serial.println(potion, HEX);
            }
            break;

            // INCREMENT
        case 0xC3:
            if (potion < 0xFF) potion++;

            // send
            hub->send(potion);
            if (dbg_DS2890) Serial.print("DS2890 : INCREMENT");
            break;

            // DECREMENT
        case 0x99:
            if (potion > 0x00) potion--;

            // send
            hub->send(potion);
            if (dbg_DS2890) Serial.print("DS2890 : DECREMENT");
            break;

        default:
            if (dbg_HINT)
            {
                Serial.print("DS2890=");
                Serial.println(done, HEX);
            }
            break;
    }

    return TRUE;
}
