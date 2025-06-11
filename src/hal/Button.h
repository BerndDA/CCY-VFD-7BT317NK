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
    bool lastState;
    unsigned long pressStartTime;
    bool longPressHandled;
    
    static constexpr unsigned long DEBOUNCE_DELAY = 50;
    static constexpr unsigned long LONG_PRESS_TIME = 500;
    
public:
    Button(uint8_t buttonPin) : pin(buttonPin), lastState(HIGH), 
                                pressStartTime(0), longPressHandled(false) {}
    
    void begin() override {
        pinMode(pin, INPUT_PULLUP);
        lastState = digitalRead(pin);
    }
    
    void update() override {
        // Simple button handling - you can enhance this with your existing logic
        bool currentState = digitalRead(pin);
        
        if (currentState != lastState) {
            if (currentState == LOW) { // Button pressed
                pressStartTime = millis();
                longPressHandled = false;
            } else { // Button released
                unsigned long pressDuration = millis() - pressStartTime;
                
                if (!longPressHandled && pressDuration < LONG_PRESS_TIME) {
                    if (eventCallback) {
                        eventCallback(ButtonEvent::SHORT_PRESS);
                    }
                }
            }
            lastState = currentState;
        }
        
        // Check for long press
        if (currentState == LOW && !longPressHandled) {
            if (millis() - pressStartTime >= LONG_PRESS_TIME) {
                longPressHandled = true;
                if (eventCallback) {
                    eventCallback(ButtonEvent::LONG_PRESS);
                }
            }
        }
    }
    
    void onButtonEvent(std::function<void(ButtonEvent)> callback) override {
        eventCallback = callback;
    }
};

#endif // BUTTON_H