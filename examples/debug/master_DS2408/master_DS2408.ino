/*
 * DS2408 8-Channel Addressable Switch
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
    byte i;
    byte present = 0;
    byte address[8];

    delay(50);

    if (!ds.search(address))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds.reset_search();
        delay(500);
        return;
    }

    Serial.print("ROM =");
    for (i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(address[i], HEX);
    }

    if (OneWire::crc8(address, 7) == address[7])
    {
        Serial.println(" - CRC OK");
    } else
    {
        Serial.println(" - CRC is not valid!");
        return;
    }

    if (address[0] == 0x29)
    {
        Serial.println("  Chip = DS2408 ");
    } else
    {
        Serial.println("Device is not a DS2408.");
        return;
    }

    uint8_t buf[13];  // Put everything in the buffer so we can compute CRC easily.
    buf[0] = 0xF0;    // Read PIO Registers
    buf[1] = 0x88;    // LSB address
    buf[2] = 0x00;    // MSB address

    present = ds.reset();
    ds.select(address);
    ds.write_bytes(buf, 3);

    Serial.print("  data = ");
    // First 3 bytes contain command, register address.
    for (uint8_t i = 0; i < 13; i++)
    {           // we need 9 bytes
        if (i > 2) buf[i] = ds.read();
        Serial.print(buf[i], HEX);
        Serial.print(" ");
    }
    //ds.read_bytes(buf+3, 10);     // 3 cmd bytes, 6 data bytes, 2 0xFF, 2 CRC16
    ds.reset();

    if (OneWire::check_crc16(buf, 11, &buf[11]))
    {
        Serial.println(" - CRC OK");
    }
    else
    {
        Serial.println(" - CRC failure in Payload");
        return;
    }
}
