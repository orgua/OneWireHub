#include "DS2423.h"

DS2423::DS2423(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 65535,  "Implementation does not cover the whole address-space");
    clearMemory();

    memset(scratchpad, static_cast<uint8_t>(0x00), PAGE_SIZE);

    for (uint8_t n = 0; n < COUNTER_COUNT; ++n)
    {
        setCounter(n,0);
    }



};

void DS2423::duty(OneWireHub * const hub)
{
    constexpr uint32_t DUMMY_32b_ZERO   = 0x00000000;
    constexpr uint32_t DUMMY_32b_ONES   = 0xFFFFFFFF;
    constexpr uint8_t ALTERNATING_10    = 0xAA;
    static uint16_t reg_TA = 0;
    static uint8_t  reg_ES = 0;
    uint16_t crc = 0;  // target_address
    uint8_t  data;
    //uint16_t memory_address_start; // not used atm, but maybe later


    uint8_t cmd;
    if (hub->recv(&cmd,1,crc))  return;

    switch (cmd)
    {
        case 0x0F:      // Write Scratchpad
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) break;

            reg_TA &= REG_TA_MASK; // TODO: can be optimized by just accessing the upper byte
            reg_ES = uint8_t(reg_TA) & PAGE_MASK;

            for (;reg_ES < PAGE_SIZE; ++reg_ES)
            {
                if (hub->recv(&scratchpad[reg_ES],1,crc)) return;
                // if one byte is not complete issue PF-bit in ES
            }
            reg_ES |= PAGE_MASK;
            crc = ~crc;
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) break;
            break;

        case 0xAA:      // read Scratchpad TODO: we don't need CRC here
            if (hub->send(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) break;
            if (hub->send(&reg_ES,2,crc)) break;
            {
                const uint8_t start     = uint8_t(reg_TA) & PAGE_MASK;
                const uint8_t length    = (reg_ES & PAGE_MASK) + uint8_t(1) - start;
                if (hub->send(&scratchpad[start],length,crc)) return;
            }
            break; // send 1s, be passive ...

        case 0x5A:      // copy scratchpad TODO: we don't need CRC here
            if (hub->recv(&data,1,crc)) break;
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[0]) break;
            if (hub->recv(&data,1,crc)) break;
            if (data != reinterpret_cast<uint8_t *>(&reg_TA)[1]) break;
            if (hub->recv(&data,1,crc)) break;
            if (data != reg_ES) break;

            reg_ES |= REG_ES_AA_MASK; // compare was successful
            // we have ~30µs to write the date
            {
                const uint8_t start     = uint8_t(reg_TA) & PAGE_MASK;
                const uint8_t length    = (reg_ES & PAGE_MASK) + uint8_t(1) - start;
                memcpy(&memory[start], &scratchpad[start], length);
            }

            while (true) // send 1s when alternating 1 & 0 after copy is complete
            {
                if (hub->send(&ALTERNATING_10)) return;
            };
            break;

        case 0xF0:      // READ MEMORY TODO: we don't need CRC here
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) break;
            reg_TA &= REG_TA_MASK; // TODO: can be optimized by just accessing the upper byte
            {
                uint8_t page  = uint8_t(reg_TA >> 5);
                uint8_t start = uint8_t(reg_TA) & PAGE_MASK;
                for (;page < PAGE_COUNT; ++page)
                {
                    const uint8_t length = PAGE_SIZE - start;
                    if (hub->send(&memory[(page*PAGE_SIZE) + start],length,crc)) return;
                    start = 0;
                };
            };
            break; // send 1s, be passive ...

        case 0xA5:      // Read Memory + Counter
            if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) return;
            reg_TA &= REG_TA_MASK; // TODO: can be optimized by just accessing the upper byte
            {
                uint8_t page  = uint8_t(reg_TA >> 5);
                uint8_t start = uint8_t(reg_TA) & PAGE_MASK;
                for (;page < PAGE_COUNT; ++page)
                {
                    const uint8_t length = PAGE_SIZE - start;
                    if (hub->send(&memory[(page*PAGE_SIZE) + start],length,crc)) return;
                    start = 0;
                    if (page > 11) // TODO: page 12 & 13 have write-counters, page 14&15 have hw-counters
                    {
                        if (hub->send(reinterpret_cast<uint8_t *>(&memcounter[page-12]),4,crc)) return;
                    }
                    else
                    {
                        if (hub->send(reinterpret_cast<const uint8_t *>(&DUMMY_32b_ONES),4,crc)) return;
                    };
                    if (hub->send(reinterpret_cast<const uint8_t *>(&DUMMY_32b_ZERO),4,crc)) return;

                    crc = ~crc;
                    if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
                    crc = 0;
                };
            };
            break;

        default:
            hub->raiseSlaveError(cmd);
    };
};


void DS2423::clearMemory(void)
{
    memset(memory, static_cast<uint8_t>(0x00), MEM_SIZE);
};

bool DS2423::writeMemory(const uint8_t* const source, const uint16_t length, const uint16_t position)
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(&memory[position],source,_length);

    return true;
};

bool DS2423::readMemory(uint8_t* const destination, const uint16_t length, const uint16_t position) const
{
    if (position >= MEM_SIZE) return false;
    const uint16_t _length = (position + length >= MEM_SIZE) ? (MEM_SIZE - position) : length;
    memcpy(destination,&memory[position],_length);
    return (_length==length);
};

void     DS2423::setCounter(uint8_t counter, uint32_t value)
{
    if (counter > COUNTER_COUNT) return;
    memcounter[counter] = value;
};

uint32_t DS2423::getCounter(uint8_t counter)
{
    if (counter > COUNTER_COUNT) return 0;
    return memcounter[counter];
};

void     DS2423::incrementCounter(uint8_t counter)
{
    if (counter > COUNTER_COUNT) return;
    if (memcounter[counter] == 0xFFFF) return;
    memcounter[counter]++;
};

void     DS2423::decrementCounter(uint8_t counter)
{
    if (counter > COUNTER_COUNT) return;
    if (memcounter[counter] == 0x0000) return;
    memcounter[counter]--;
};
