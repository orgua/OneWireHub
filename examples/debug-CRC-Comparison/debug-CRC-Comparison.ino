/*
 *    Test-Code for faster CRC-Calculations, mode 0xC001 for little endian
 *
 *    Output (Atmega328P@16MHz):
 *
 *    Test-Code for faster CRC-Calculations<\r><\n>
 *    CRCing 254 bytes <\r><\n>
 *    Var 1A took 2476 us, got 127B<\r><\n>
 *    Var 1B took 2928 us, got 127B<\r><\n>
 *    Var 1C took 1912 us, got 127B<\r><\n>
 *    Var 2A took 1384 us, got 127B<\r><\n>
 *    Var 2B took 1848 us, got 127B<\r><\n>
 *    Var 2C took 1500 us, got 127B<\r><\n>
 *    Var 2D took 1912 us, got 127B<\r><\n>
 *    Benchmark: 1ms got us 293 * (micros(), 32bit check, 32bit increment)
 *
 *    Result:
 *
 *    - Var2 is faster for longer CRCing >=1byte-wise
 *       - takes from 5.1 to 7.0 µs/byte dependant from array-length
 *    - Var1 is slower, but can be fragmented in 8 equal parts
 *       - ~0.9 µs per Bit-Step for Var1C
 */

void setup()
{
    Serial.begin(115200);
    Serial.println("Test-Code for faster CRC-Calculations");

    constexpr uint8_t li[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed "
            "do eiusmod tempor incididunt ut labore et dolore magna "
            "aliqua. Ut enim ad minim veniam, quis nostrud exercitation "
            "ullamco laboris nisi ut aliquip ex ea commodo consequat. Duis "
            "aute irure dolor";
    constexpr uint8_t li_size = sizeof(li);

    Serial.print("CRCing ");
    Serial.print(li_size);
    Serial.println(" bytes ");

    uint32_t time_start, time_stop;
    uint16_t crc;

    /// Start Var 1A
    // void v1A_crc16_update(uint8_t b)

    v1A_crc16_reset();
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        v1A_crc16_update(li[bytepos]);
    time_stop = micros();

    Serial.print("Var 1A took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(v1A_crc16_get(), HEX);
    Serial.flush();

    /// Start Var 1B
    // bool v1B_crc16_update(const uint8_t databyte, uint16_t &crc16)

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        v1B_crc16_update(li[bytepos], crc);
    time_stop = micros();

    Serial.print("Var 1B took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 1C
    // uint16_t v1C_crc16_update(uint8_t databyte, uint16_t crc16)

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        crc = v1C_crc16_update(li[bytepos], crc);
    time_stop = micros();

    Serial.print("Var 1C took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2A
    // uint16_t v2A_crc16(const uint8_t addr[], const uint8_t len)

    crc = 0;
    time_start = micros();
    crc = v2A_crc16(li, li_size);
    time_stop = micros();

    Serial.print("Var 2A took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2B
    // uint16_t v2B_crc16(uint16_t crc, const uint8_t addr, const uint8_t len)

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        crc = v2B_crc16(crc,li[bytepos], 1);
    time_stop = micros();

    Serial.print("Var 2B took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2C
    // uint16_t v2C_crc16(uint16_t crc, uint8_t value) // TODO: further tuning with asm

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        crc = v2C_crc16(crc, li[bytepos]);
    time_stop = micros();

    Serial.print("Var 2C took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2D
    // void v2D_crc16(uint16_t &crc, uint8_t value) // TODO: further tuning with asm

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        v2D_crc16(crc, li[bytepos]);
    time_stop = micros();

    Serial.print("Var 2D took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();




    /// timer test
    uint16_t counter = 0;
    time_stop = micros() + 1000;
    while (micros() < time_stop)
    {
        counter++;
    }
    Serial.print("Benchmark: 1ms got us ");
    Serial.print(counter);
    Serial.print(" * (micros(), 32bit check, 32bit increment)");
}

void loop()
{

}


/// Variant 1A - old code

static uint16_t crc16;

void v1A_crc16_reset(void)
{
    crc16 = 0;
}

void v1A_crc16_update(uint8_t b)
{
    for (uint8_t j = 0; j < 8; ++j)
    {
        const bool mix = (static_cast<uint8_t>(crc16) ^ b) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)
            crc16 ^= static_cast<uint16_t>(0xA001);
        b >>= 1;
    }
}

uint16_t v1A_crc16_get(void)
{
    return crc16;
}


/// Variant 1B - break up the loop for parallel sending and try pass by reference

bool v1B_crc16_update(uint8_t _databyte, uint16_t &crc16)
{
    //_error = ONEWIRE_NO_ERROR;
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        //sendBit((bitmask & databyte) ? 1 : 0);

        uint8_t mix = ((uint8_t) crc16 ^ _databyte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        _databyte >>= 1;

        //if (_error) return false;
    }
    return true;
}

/// Variant 1C - pass by value is much faster

uint16_t v1C_crc16_update(uint8_t databyte, uint16_t crc16)
{
    //_error = ONEWIRE_NO_ERROR;
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        //sendBit((bitmask & databyte) ? 1 : 0);

        uint8_t mix = ((uint8_t) crc16 ^ databyte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        databyte >>= 1;

        //if (_error) return false;
    }
    return crc16;
}

/// Variant 2A - onewire-lib original

uint16_t v2A_crc16(const uint8_t addr[], const uint8_t len)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    uint16_t crc = 0; // initvalue

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = addr[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
};


/// Variant 2B - onewire-lib callable one byte at once

uint16_t v2B_crc16(uint16_t crc, const uint8_t addr, const uint8_t len)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = addr;
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
};

/// Variant 2C - transformed for speed

uint16_t v2C_crc16(uint16_t crc, uint8_t value) // TODO: further tuning with asm
{
    static const uint8_t oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if (oddparity[value & 0x0F] ^ oddparity[value >> 4])   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
};

/// var 2D - call by reference

void v2D_crc16(uint16_t &crc, uint8_t value) // TODO: further tuning with asm
{
    static const uint8_t oddparity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if (oddparity[value & 0x0F] ^ oddparity[value >> 4])   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
};

