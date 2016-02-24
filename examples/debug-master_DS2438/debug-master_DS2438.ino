// OneWire DS2438 Example
//
// You need : https://github.com/PaulStoffregen/OneWire

#include <OneWire.h>

OneWire ds(8);  // on pin 8 (a 4.7K resistor is necessary)

void setup(void)
{
    Serial.begin(115200);
}

void loop(void)
{
    byte i;
    byte present = 0;
    byte type_s;
    byte data[12];
    byte address[8];

    if (!ds.search(address))
    {
        Serial.println("No more addresses.");
        Serial.println();
        ds.reset_search();
        delay(250);
        return;
    }

    Serial.print("ROM =");
    for (i = 0; i < 8; i++)
    {
        Serial.write(' ');
        Serial.print(address[i], HEX);
    }

    if (OneWire::crc8(address, 7) != address[7])
    {
        Serial.println("CRC is not valid!");
        return;
    }
    Serial.println();

    // the first ROM byte indicates which chip
    switch (address[0])
    {
        case 0x26:
            Serial.println("  Chip = DS2438");
            type_s = 1;
            break;
        default:
            Serial.println("Device is not recognized.");
            return;
    }

    ds.reset();
    ds.select(address);
    ds.write(0x44, 1);        // start T_conversion, with parasite power on at the end
    //ds.write(0xB4, 1);        // start V_conversion, with parasite power on at the end

    delay(1000);     // maybe 750ms is enough, maybe not

    present = ds.reset();
    ds.select(address);
    ds.write(0xBE);         // Read Scratchpad
    ds.write(0x00);         // request Page 0, measurements
    Serial.print("  Data = ");
    Serial.print(present, HEX);
    Serial.print(" :: ");
    for (i = 0; i < 9; i++)
    {           // we need 9 bytes
        data[i] = ds.read();
        Serial.print(data[i], HEX);
        Serial.print(" ");
    }
    uint8_t crc = OneWire::crc8(data, 8);
    if (crc == data[8])
    {
        Serial.println(" CRC OK ");
    }
    else
    {
        Serial.print(" CRC ERROR - expected ");
        Serial.println(crc, HEX);
    }

    // Convert the data to actual values
    int16_t raw = (data[2] << 8) | data[1];
    float temperature = raw / 256.0; // degC
    raw = ((data[4] << 8) & 0x03) | data[3];
    float voltage = raw / 100.0;   // V
    raw = (data[6] << (8 + 5)) | (data[5] << 5);
    float current = raw / 1024.0;   // A

    // display data
    Serial.print("  Temp = ");
    Serial.print(temperature);
    Serial.print(" degC, Voltage = ");
    Serial.print(voltage);
    Serial.print(" V, Current = ");
    Serial.print(current);
    Serial.println(" A");
}
