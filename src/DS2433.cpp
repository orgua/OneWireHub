#include "DS2433.h"

DS2433::DS2433(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    for (int i = 0; i < sizeof(memory); ++i)
    {
        memory[i] = 0x00;
    };

};

bool DS2433::duty(OneWireHub *hub)
{
    static uint16_t register_ta; // contains TA1, TA2
    static uint8_t  register_es = 31;  // E/S register

    uint8_t  mem_counter = 0; // offer feedback with PF-bit
    uint16_t crc = 0;
    uint8_t  b;

    uint8_t cmd = hub->recv();
    if (hub->getError())  return false;

    switch (cmd)
    {
        // WRITE SCRATCHPAD COMMAND
        case 0x0F:
            crc = crc16(0x0F,0);
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[0] = b;
            register_es = b & uint8_t(0b00011111); // register-offset
            crc = crc16(b,crc);

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[1] = b & uint8_t(0b1);
            crc = crc16(b,crc);

            for (uint16_t i = 0; i < (32-register_es); ++i) // model of the 32byte scratchpad, directly to memory
            {
                b = hub->recv();
                if (hub->getError())
                {
                    // set the PF-Flag if error occured in the middle of the byte
                    if (hub->clearError() != Error::FIRST_BIT_OF_BYTE_TIMEOUT) register_es |= 0b00100000;
                    break;
                };
                memory[register_ta + i] = b;
                crc = crc16(b,crc);
                mem_counter++;
            };
            if ((register_es + mem_counter) > 31) mem_counter = uint8_t(31) - register_es;
            register_es += mem_counter; // store current pointer

            if (register_es != 0b00011111) break;

            crc = ~crc; // normally crc16 is sent ~inverted
            hub->send(uint8_t(reinterpret_cast<uint8_t *>(&crc)[0]));
            if (hub->getError())  return false;
            hub->send(reinterpret_cast<uint8_t *>(&crc)[1]);

            break;

            // COPY SCRATCHPAD
        case 0x55:
            b = hub->recv(); // TA1
            if (hub->getError())  return false;
            //if (b != reinterpret_cast<uint8_t *>(&register_ta)[0]) break;

            b = hub->recv(); // TA2
            if (hub->getError())  return false;
            //if (b != reinterpret_cast<uint8_t *>(&register_ta)[1]) break;

            b = hub->recv(); // ES
            if (hub->getError())  return false;
            //if (b != register_es) break;

            register_es |= 0b10000000;

            // TODO: maybe implement a real scratchpad
            delayMicroseconds(5000); // simulate writing
            hub->extendTimeslot();
            hub->sendBit(1);
            hub->clearError();
            hub->extendTimeslot();
            while (1) // send alternating 1 & 0 after copy is complete
            {
                hub->send(0b10101010);
                if (hub->getError()) break;
            };

            break;

            // READ SCRATCHPAD COMMAND
        case 0xAA:
            // Adr1
            hub->send(reinterpret_cast<uint8_t *>(&register_ta)[0]);
            if (hub->getError())  return false;

            // Adr2
            hub->send(reinterpret_cast<uint8_t *>(&register_ta)[1]);
            if (hub->getError())  return false;

            // ES
            hub->send(register_es);
            if (hub->getError())  return false;

            hub->extendTimeslot();

            // data
            for (uint8_t i = (uint8_t(register_ta) & uint8_t(0b00011111)); i < 32; ++i) // model of the 32byte scratchpad
            {
                const uint16_t mem_start = (register_ta & ~uint16_t(0b00011111));
                hub->send(memory[mem_start + i]);
                if (hub->getError()) break;
            };

            if (hub->getError()) break;

            // datasheed says we should return all 1s, send(255), till reset
            while (1)
            {
                hub->send(255);
                if (hub->getError()) break;
            };

            break;

            // READ MEMORY
        case 0xF0:
            // Adr1
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[0] = b;

            // Adr2
            b = hub->recv();
            if (hub->getError())  return false;
            reinterpret_cast<uint8_t *>(&register_ta)[1] = b;

            // data
            for (uint16_t i = register_ta; i < 512; ++i) // model of the 32byte scratchpad
            {
                hub->send(memory[i]);
                if (hub->getError()) break;
            };

            if (hub->getError()) break;

            // datasheed says we should return all 1s, send(255), till reset
            while (1)
            {
                hub->send(255);
                if (hub->getError()) break;
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
    return !(hub->getError());
};
