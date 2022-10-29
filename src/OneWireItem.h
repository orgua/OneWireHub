#ifndef ONEWIREHUB_ONEWIREITEM_H
#define ONEWIREHUB_ONEWIREITEM_H

#include "OneWireHub.h"

class OneWireItem
{
public:

    OneWireItem();

    ~OneWireItem() = default; // TODO: detach if deleted before hub

    OneWireItem(const OneWireItem& owItem) = delete;             // disallow copy constructor
    OneWireItem(OneWireItem&& owItem) = default;               // default  move constructor
    OneWireItem& operator=(OneWireItem& owItem) = delete;        // disallow copy assignment
    OneWireItem& operator=(const OneWireItem& owItem) = delete;  // disallow copy assignment
    OneWireItem& operator=(OneWireItem&& owItem) = delete;       // disallow move assignment

    virtual void duty(OneWireHub * hub) = 0;
};

#endif //ONEWIREHUB_ONEWIREITEM_H
