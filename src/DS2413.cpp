#include "DS2413.h"


DS2413::DS2413(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    pin_state[0] = false;
    pin_latch[0] = false;
    pin_state[1] = false;
    pin_latch[1] = false;
}

bool DS2413::duty(OneWireHub *hub)
{
    uint8_t cmd = hub->recv();
    uint8_t data = 0;

    switch (cmd)
    {
        case 0x5A: // PIO ACCESS WRITE
            data = ~hub->recv(); // Write inverse
            hub->send(data); // send inverted form for safety

            setLatch(0, data & static_cast<uint8_t>(0x01)); // A
            setLatch(1, data & static_cast<uint8_t>(0x02)); // B
            setState(0, ~(data & static_cast<uint8_t>(0x01)));
            setState(1, ~(data & static_cast<uint8_t>(0x01)));
            break;


        case 0xF5: // PIO ACCESS READ
            data = 0;

            if (pin_state[0])  data = data | static_cast<uint8_t>(0x01);
            if (!pin_latch[0]) data = data | static_cast<uint8_t>(0x02);
            if (pin_state[1])  data = data | static_cast<uint8_t>(0x04);
            if (!pin_latch[1]) data = data | static_cast<uint8_t>(0x08);

            data = data | (~data << 4);
            hub->send(data);

            break;

        default:
            hub->raiseSlaveError(cmd);
            return false;
    }

    return true;
}