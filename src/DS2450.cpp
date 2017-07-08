#include "DS2450.h"

DS2450::DS2450(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) :
        OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
    static_assert(sizeof(memory) < 256,  "Implementation does not cover the whole address-space");
    clearMemory();
}

void DS2450::duty(OneWireHub * const hub)
{
    uint16_t reg_TA, crc = 0; // target address
    uint8_t  cmd;

    if (hub->recv(&cmd,1,crc))  return;
    if (hub->recv(reinterpret_cast<uint8_t *>(&reg_TA),2,crc)) return;

    switch (cmd)
    {
        case 0xAA: // READ MEMORY

            while(reg_TA < MEM_SIZE)
            {
                const uint8_t length = PAGE_SIZE - (uint8_t(reg_TA) & PAGE_MASK);
                if (hub->send(&memory[reg_TA], length, crc)) return;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) return;

                // prepare next page-readout
                reg_TA += length;
                crc = 0;
            }
            break;

        case 0x55: // write memory (only page 1&2 allowed)

            while(reg_TA < MEM_SIZE)
            {
                uint8_t data;
                if (hub->recv(&data, 1, crc))   break;

                crc = ~crc; // normally crc16 is sent ~inverted
                if (hub->send(reinterpret_cast<uint8_t *>(&crc), 2)) break;

                if (hub->send(&data, 1))        break;
                if (reg_TA >= PAGE_SIZE)        memory[reg_TA] = data; // write data, page 0 is off limits

                crc = ++reg_TA; // prepare next address-readout: load new TA into crc
            }
            correctMemory();
            break;

        case 0x3C: // convert, starts adc

            // received reg_TA contains: input select mask (not important) and read out control byte
            // in reality master can now set registers of potentiometers to 0x0000 or 0xFFFF to track changes
            crc = ~crc; // normally crc16 is sent ~inverted
            if (hub->send(reinterpret_cast<uint8_t *>(&crc),2)) return;
            // takes max 5.3 ms for 16 bit ( 4 CH * 16 bit * 80 us + 160 us per request = 5.3 ms )
            noInterrupts();
            hub->sendBit(false); // still converting....
            interrupts();
            break; // finished conversion: send 1, is passive ...

        default:

            hub->raiseSlaveError(cmd);
    }
}

void DS2450::clearMemory(void)
{
    constexpr uint8_t value_x00 = 0;
    memset(memory, value_x00, MEM_SIZE);

    // set power on defaults
    for (uint8_t adc = 0; adc < POTI_COUNT; ++adc)
    {
        // CONTROL/STATUS DATA
        memory[(1*PAGE_SIZE) + (adc*2) + 0] = 0x00; // 16bit
        memory[(1*PAGE_SIZE) + (adc*2) + 1] = 0x8C; // enable POR, Alarm enable high / low
        // alarm settings
        memory[(2*PAGE_SIZE) + (adc*2) + 1] = 0xFF; // high threshold max
    }
}

void DS2450::correctMemory(void)
{
    for (uint8_t adc = 0; adc < POTI_COUNT; ++adc)
    {
        //// control / status data
        /// byte 0,2,4,6
        // bit 0:3 -> RC3 sets resolution of the ADCs. 1to15bits and 0 for 16 bits. MSB aligned
        memory[(1*PAGE_SIZE) + (adc*2)] &= 0b11001111; // bit 4:5 must be always zero
        // bit 6 -> output control: set 0 for enablesd transistors
        // bit 7 -> output enable: set 0 for ADC,
        /// byte 1,3,5,7
        // bit 0 -> IR sets input voltage: 0 for 2.55 V, 1 for 5.1 V
        memory[(1*PAGE_SIZE) + (adc*2) + 1] &= 0b10111101; // bit 1&6 -> always zero
        // bit 2:3 -> enable alarm search low, high
        // bit 4:5 -> alarm flag for low, high
        // bit 7 -> power on reset, must be written 0 by master
    }
}

bool DS2450::setPotentiometer(const uint16_t p1, const uint16_t p2, const uint16_t p3, const uint16_t p4)
{
    setPotentiometer(0, p1);
    setPotentiometer(1, p2);
    setPotentiometer(2, p3);
    setPotentiometer(3, p4);
    return true;
}

bool DS2450::setPotentiometer(const uint8_t channel, const uint16_t value)
{
    if (channel >= POTI_COUNT) return false;
    uint8_t LByte = static_cast<uint8_t>(value>>0) & static_cast<uint8_t>(0xFF);
    uint8_t HByte = static_cast<uint8_t>(value>>8) & static_cast<uint8_t>(0xFF);
    memory[(2*channel)  ] = LByte;
    memory[(2*channel)+1] = HByte;
    correctMemory();
    return true; // TODO: check with alarm settings p2, and raise alarm, also check when data is written
}

uint16_t DS2450::getPotentiometer(const uint8_t channel) const
{
    if (channel >= POTI_COUNT) return 0;
    uint16_t value;
    value  = memory[(2*channel)+1]<<8;
    value |= memory[(2*channel)  ];
    return value;
}


