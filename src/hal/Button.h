// hal/Button.h
#ifndef BUTTON_H
#define BUTTON_H

#include "IButton.h"
#include <Arduino.h>

class Button : public IButton {
private:
    uint8_t pin;
    std::function<void(ButtonEvent)> eventCallback;
    
    // Button state tracking
    bool currentState;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long pressStartTime;
    bool buttonPressed;
    bool longPressHandled;
    bool longPressHoldActive;
    
    static constexpr unsigned long DEBOUNCE_DELAY = 50;
    static constexpr unsigned long LONG_PRESS_TIME = 500;
    static constexpr unsigned long LONG_PRESS_HOLD_INTERVAL = 800;
    
public:
    Button(uint8_t buttonPin);
    
    void begin() override;
    void update() override;
    void onButtonEvent(std::function<void(ButtonEvent)> callback) override;
    
private:
    void handleButtonPress();
    void handleButtonRelease();
    void checkLongPress();
};

#endif // BUTTON_H