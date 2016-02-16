/*
 *    Example-Code that emulates a DS2413-Sensor --> not tested
 *    --> attach sensors as needed
 *    Tested with https://github.com/PaulStoffregen/OneWire on the other side as Master
 */

#include "OneWireHub.h"
#include "DS2413.h"  // Dual channel addressable switch
#include "TimerOne.h"

const uint8_t BUTTON_PIN    = 2;
const uint8_t OneWire_PIN   = 8;

int RelayCnt = 0;
#define RelayTime 5

class MovmentSensor : public DS2413
{
public:
    MovmentSensor(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7);

    void ReadState();
};

MovmentSensor::MovmentSensor(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : DS2413(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

void MovmentSensor::ReadState()
{
    this->AState = RelayCnt != 0;
    // Serial.println( this->AState );
}

OneWireHub  hub     = OneWireHub(OneWire_PIN); // pin D8
auto        sensor  = MovmentSensor(0x3A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00);

void setup()
{
    // Debug
    Serial.begin(115200);
    Serial.println("OneWire-Hub Movement-Sensor DS2413");
    // Setup the button
    pinMode(BUTTON_PIN, INPUT);
    // Activate internal pull-up
    digitalWrite(BUTTON_PIN, HIGH);

    attachInterrupt(0, DoRelay, FALLING);

    // Work - Dual channel addressable switch

    // Setup OneWire
    hub.attach(sensor);

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(DoTimer);

    Serial.println("config done");
}

void loop()
{
    // put your main code here, to run repeatedly:
    hub.waitForRequest(false);
}

void DoRelay()
{
    RelayCnt = RelayTime;
}

void DoTimer()
{
    if (RelayCnt > 0)
    {
        RelayCnt--;
    }
}
