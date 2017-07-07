/*
 *    Test-Code for faster CRC-Calculations, mode 0xC001 for little endian
 *
 *    Output (Atmega328P@16MHz):
 *
 *    Test-Code for faster CRC-Calculations<\r><\n>
 *    CRC-ing 254 bytes <\r><\n>
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

#include <util/crc16.h>

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

    Serial.print("CRC-ing ");
    Serial.print(li_size);
    Serial.println(" bytes ");

    uint32_t time_start, time_stop;
    uint16_t crc;

    /// Start Var 1A //////////////////////////////////////////////////

    v1A_crc16_reset();
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        v1A_crc16_update(li[bytePos]);
    time_stop = micros();

    Serial.print("Var 1A took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(v1A_crc16_get(), HEX);
    Serial.flush();

    /// Start Var 1B //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        v1B_crc16_update(li[bytePos], crc);
    time_stop = micros();

    Serial.print("Var 1B took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 1C //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = v1C_crc16_update(li[bytePos], crc);
    time_stop = micros();

    Serial.print("Var 1C took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2A //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    crc = v2A_crc16(li, li_size);
    time_stop = micros();

    Serial.print("Var 2A took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2B //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = v2B_crc16(crc,li[bytePos], 1);
    time_stop = micros();

    Serial.print("Var 2B took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2C //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = v2C_crc16(crc, li[bytePos]);
    time_stop = micros();

    Serial.print("Var 2C took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2D //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        v2D_crc16(crc, li[bytePos]);
    time_stop = micros();

    Serial.print("Var 2D took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 2E //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = v2E_crc16(crc, li[bytePos]);
    time_stop = micros();

    Serial.print("Var 2E took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();


    /// Start Var 3A //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = v3A_crc16(crc, li[bytePos]);
    time_stop = micros();

    Serial.print("Var 3A took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

    /// Start Var 3B //////////////////////////////////////////////////

    crc = 0;
    time_start = micros();
    for (uint8_t bytePos = 0; bytePos < li_size; ++bytePos)
        crc = _crc16_update(crc, li[bytePos]);
    time_stop = micros();

    Serial.print("Var 3B took ");
    Serial.print(time_stop - time_start);
    Serial.print(" us, got ");
    Serial.println(crc, HEX);
    Serial.flush();

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

bool v1B_crc16_update(uint8_t dataByte, uint16_t &crc16)
{
    //_error = ONEWIRE_NO_ERROR;
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        //sendBit((bitMask & dataByte) ? 1 : 0);

        uint8_t mix = ((uint8_t) crc16 ^ dataByte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        dataByte >>= 1;

        //if (_error) return false;
    }
    return true;
}

/// Variant 1C - pass by value is much faster

uint16_t v1C_crc16_update(uint8_t dataByte, uint16_t crc16)
{
    //_error = ONEWIRE_NO_ERROR;
    for (uint8_t counter = 0; counter < 8; ++counter)
    {
        //sendBit((bitMask & dataByte) ? 1 : 0);

        uint8_t mix = ((uint8_t) crc16 ^ dataByte) & static_cast<uint8_t>(0x01);
        crc16 >>= 1;
        if (mix)  crc16 ^= static_cast<uint16_t>(0xA001);
        dataByte >>= 1;

        //if (_error) return false;
    }
    return crc16;
}

/// Variant 2A - onewire-lib original

uint16_t v2A_crc16(const uint8_t address[], const uint8_t len)
{
    static const uint8_t oddParity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    uint16_t crc = 0; // init value

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = address[i];
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddParity[cdata & 0x0F] ^ oddParity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
}


/// Variant 2B - onewire-lib callable one byte at once, untouched

uint16_t v2B_crc16(uint16_t crc, const uint8_t value, const uint8_t len)
{
    static const uint8_t oddParity[16] =
            {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

    for (uint8_t i = 0; i < len; ++i)
    {
        // Even though we're just copying a byte from the input,
        // we'll be doing 16-bit computation with it.
        uint16_t cdata = value;
        cdata = (cdata ^ crc) & static_cast<uint16_t>(0xff);
        crc >>= 8;

        if (oddParity[cdata & 0x0F] ^ oddParity[cdata >> 4])
            crc ^= 0xC001;

        cdata <<= 6;
        crc ^= cdata;
        cdata <<= 1;
        crc ^= cdata;
    }

    return crc;
}

/// Variant 2C - call one by one (byte), transformed for speed

uint16_t v2C_crc16(uint16_t crc, uint8_t value)
{
    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
}

/// var 2D - Test call by reference --> not a good idea

void v2D_crc16(uint16_t &crc, uint8_t value)
{
    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    value = (value ^ static_cast<uint8_t>(crc));
    crc >>= 8;
    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
}


/// Variant 2E - more tuning with ASM-compare

uint16_t v2E_crc16(uint16_t crc, uint8_t value)
{
    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
    //value = (value ^ static_cast<uint8_t>(crc));
    value ^= static_cast<uint8_t>(crc);
    crc >>= static_cast<uint8_t>(8);
    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
    //if (oddParity[static_cast<uint8_t>(value & static_cast<uint8_t>(0x0F))] ^ oddParity[value >> 4])   crc ^= 0xC001; // --> no difference
    //if (oddParity[value & 0x0F] ^ oddParity[static_cast<uint8_t>(value >> static_cast<uint8_t>(4))])   crc ^= 0xC001; // --> no difference
    //if (static_cast<uint8_t>(oddParity[value & 0x0F] ^ oddParity[value >> 4]))   crc ^= 0xC001; // --> no difference
    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
    crc ^= cdata;
    crc ^= (static_cast<uint16_t>(cdata) << 1);
    return crc;
}

/*   Assembly of var 2D
 *
 * 	    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
	    value = (value ^ static_cast<uint8_t>(crc));
  b0:	42 27       	eor	r20, r18
	    crc >>= 8;
  b2:	23 2f       	mov	r18, r19
  b4:	33 27       	eor	r19, r19
	    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
  b6:	a4 2f       	mov	r26, r20
  b8:	af 70       	andi	r26, 0x0F	; 15
  ba:	b0 e0       	ldi	r27, 0x00	; 0
  bc:	a0 50       	subi	r26, 0x00	; 0
  be:	bf 4f       	sbci	r27, 0xFF	; 255
  c0:	50 e0       	ldi	r21, 0x00	; 0
  c2:	fa 01       	movw	r30, r20
  c4:	84 e0       	ldi	r24, 0x04	; 4
  c6:	f5 95       	asr	r31
  c8:	e7 95       	ror	r30
  ca:	8a 95       	dec	r24
  cc:	e1 f7       	brne	.-8      	; 0xc6 <main+0x30>
  ce:	e0 50       	subi	r30, 0x00	; 0
  d0:	ff 4f       	sbci	r31, 0xFF	; 255
  d2:	9c 91       	ld	r25, X
  d4:	80 81       	ld	r24, Z
  d6:	98 17       	cp	r25, r24
  d8:	21 f0       	breq	.+8      	; 0xe2 <main+0x4c>
  da:	81 e0       	ldi	r24, 0x01	; 1
  dc:	28 27       	eor	r18, r24
  de:	80 ec       	ldi	r24, 0xC0	; 192
  e0:	38 27       	eor	r19, r24
	    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
  e2:	86 e0       	ldi	r24, 0x06	; 6
  e4:	44 0f       	add	r20, r20
  e6:	55 1f       	adc	r21, r21
  e8:	8a 95       	dec	r24
  ea:	e1 f7       	brne	.-8      	; 0xe4 <main+0x4e>
	    crc ^= cdata;
	    crc ^= (static_cast<uint16_t>(cdata) << 1);
  ec:	ca 01       	movw	r24, r20
  ee:	88 0f       	add	r24, r24
  f0:	99 1f       	adc	r25, r25
	    static const uint8_t oddParity[16] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};
	    value = (value ^ static_cast<uint8_t>(crc));
	    crc >>= 8;
	    if (oddParity[value & 0x0F] ^ oddParity[value >> 4])   crc ^= 0xC001;
	    uint16_t cdata = (static_cast<uint16_t>(value) << 6);
	    crc ^= cdata;
  f2:	84 27       	eor	r24, r20
  f4:	95 27       	eor	r25, r21
	    crc ^= (static_cast<uint16_t>(cdata) << 1);
  f6:	82 27       	eor	r24, r18
  f8:	93 27       	eor	r25, r19

 */

/// Variant 2E - AVR Libc-Reference

uint16_t v3A_crc16(uint16_t crc, uint8_t value)
{
    crc ^= value;
    for (uint8_t i = 0; i < 8; ++i)
    {
        if (crc & 1)    crc = (crc >> 1) ^ 0xA001;
        else            crc = (crc >> 1);
    }

    return crc;
}