#include "DellAC.h"

DellAC::DellAC(uint8_t ID1, uint8_t ID2, uint8_t ID3, uint8_t ID4, uint8_t ID5, uint8_t ID6, uint8_t ID7) : OneWireItem(ID1, ID2, ID3, ID4, ID5, ID6, ID7){}

//bool DellAC::duty(OneWireHub *hub)
void DellAC::duty(OneWireHub * const hub)

{

    uint8_t cmd,data,tmp;
    hub->recv(&cmd);

    switch (cmd)
    {
        // READ MEMORY
        case 0xF0:
            // Adr1
            hub->recv(&tmp);
            // Adr2
            hub->recv(&tmp);
            hub->send(chargerData, 4);
            break;

        default:
            hub->raiseSlaveError(cmd);
            break;
    }
//    return true;
}
