/*
 * DS2408 8-Channel Addressable Switch, this is the "Fast" version, omitting all the serial while talking to the sensor
 *
 * You need : https://github.com/PaulStoffregen/OneWire
 */

#include <OneWire.h>

OneWire ds(8);  // on pin 8

void setup(void)
{
    Serial.begin(115200);
}

void loop(void)
{
    uint8_t address[8];

    delay(10);

    if (!ds.search(address))
    {
        Serial.println("No more addresses.");
        Serial.println();
        Serial.flush();
        ds.reset_search();
        delay(500);
        return;
    }

    uint8_t buf[13];  // Put everything in the buffer so we can compute CRC easily.
    buf[0] = 0xF0;    // Read PIO Registers
    buf[1] = 0x88;    // LSB address
    buf[2] = 0x00;    // MSB address

    ds.reset();
    ds.select(address);
    ds.write_bytes(buf, 3);

    for (uint8_t i = 3; i < 13; i++)
        buf[i] = ds.read();

    ds.reset();

    Serial.print("  data = ");
    for (uint8_t i = 0; i < 13; i++)
    {
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }

    if (OneWire::check_crc16(buf, 11, &buf[11]))
    {
        Serial.println(" - CRC OK");
    }
    else
    {
        Serial.println(" - CRC failure in Payload");
    }
    Serial.flush();
}
