// hal/IButton.h
#ifndef IBUTTON_H
#define IBUTTON_H

#include <functional>
#include "app/states/State.h"

class IButton {
public:
    virtual ~IButton() = default;
    
    virtual void begin() = 0;
    virtual void update() = 0;
    virtual void onButtonEvent(std::function<void(ButtonEvent)> callback) = 0;
};
#endif // IBUTTON_H