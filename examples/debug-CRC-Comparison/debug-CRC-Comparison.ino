/*
 *    Test-Code for faster CRC-Calculations, mode 0xC001 for little endian
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

    /// Start Var 1

    time_start = micros();
    v1_crc16_reset();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        v1_crc16_update(li[bytepos]);
    time_stop = micros();

    Serial.print("Var 1 took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(v1_crc16_get(), HEX);
    Serial.flush();

    /// Start Var 2

    time_start = micros();
    crc = v2_crc16(li, li_size);
    time_stop = micros();

    Serial.print("Var 2 took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2B

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        crc = v3_crc16(crc,li[bytepos], 1);
    time_stop = micros();

    Serial.print("Var 3 took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 3

    crc = 0;
    time_start = micros();
    for (uint8_t bytepos = 0; bytepos < li_size; ++bytepos)
        crc = v4_crc16(crc, li[bytepos]);
    time_stop = micros();

    Serial.print("Var 4 took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// timer
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


/// Variant 1 - old code

static uint16_t crc16;

void v1_crc16_reset(void)
{
    crc16 = 0;
}

void v1_crc16_update(uint8_t b)
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

uint16_t v1_crc16_get(void)
{
    return crc16;
}


/// Variant 2 - onewire-lib

uint16_t v2_crc16(const uint8_t addr[], const uint8_t len)
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

/// Variant 3 - onewire-lib callable one byte at once

uint16_t v3_crc16(uint16_t crc, const uint8_t addr, const uint8_t len)
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

/// Variant 4 - transformed for speed

uint16_t v4_crc16(uint16_t crc, uint8_t value)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

        value = (value ^ static_cast<uint8_t>(crc));
        crc >>= 8;

        if (oddparity[value & 0x0F] ^ oddparity[value >> 4])
            crc ^= 0xC001;

        uint16_t cdata = (static_cast<uint16_t>(value) << 6);
        crc ^= cdata;
        crc ^= (static_cast<uint16_t>(cdata) << 1);

    return crc;
};

/// var 4

// 1540 µs
uint16_t v31_crc16(uint16_t crc, const uint8_t value)
{
    static const uint8_t oddparity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

        uint16_t cdata = value;
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddparity[cdata & 0x0F] ^ oddparity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;

    return crc;
};