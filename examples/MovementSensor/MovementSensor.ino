#include "OneWireHub.h"
#include "DS2413.h"  // Dual channel addressable switch
#include "TimerOne.h"

#define BUTTON_PIN 2
#define OneWire_PIN 8

#define RelayTime 5

OneWireHub *hub = 0;

int RelayCnt = 0;

class MovmentSensor : public DS2413
{
public:
    MovmentSensor(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7);

    void ReadState();
};

MovmentSensor::MovmentSensor(byte ID1, byte ID2, byte ID3, byte ID4, byte ID5, byte ID6, byte ID7) : DS2413(ID1, ID2, ID3, ID4, ID5, ID6, ID7)
{
}

void MovmentSensor::ReadState()
{
    this->AState = RelayCnt != 0;
    // Serial.println( this->AState );
}

MovmentSensor *fMS;

void setup()
{
    // Debug
    Serial.begin(9600);

    // Setup the button
    pinMode(BUTTON_PIN, INPUT);
    // Activate internal pull-up
    digitalWrite(BUTTON_PIN, HIGH);

    attachInterrupt(0, DoRelay, FALLING);

    // Work - Dual channel addressable switch
    fMS = new MovmentSensor(0x3A, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00);

    // Setup OneWire
    hub = new OneWireHub(OneWire_PIN);
    hub->elms[0] = fMS;
    hub->calck_mask();

    Timer1.initialize(1000000);
    Timer1.attachInterrupt(DoTimer);
}

void loop()
{
    // put your main code here, to run repeatedly:
    hub->waitForRequest(false);
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
