// hal/Button.cpp - Updated with proper release detection
#include "Button.h"

Button::Button(uint8_t buttonPin) 
    : pin(buttonPin), 
      currentState(HIGH), 
      lastState(HIGH),
      lastDebounceTime(0),
      pressStartTime(0),
      buttonPressed(false),
      longPressHandled(false),
      longPressHoldActive(false) {
}

void Button::begin() {
    pinMode(pin, INPUT_PULLUP);
    currentState = digitalRead(pin);
    lastState = currentState;
}

void Button::update() {
    // Read the button state
    bool reading = digitalRead(pin);
    
    // Check if button state has changed
    if (reading != lastState) {
        lastDebounceTime = millis();
    }
    
    // If enough time has passed, accept the new state
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
        // If the button state has changed
        if (reading != currentState) {
            currentState = reading;
            
            if (currentState == LOW) {
                // Button pressed
                handleButtonPress();
            } else {
                // Button released
                handleButtonRelease();
            }
        }
    }
    
    // Check for long press while button is held
    if (buttonPressed && !longPressHandled) {
        checkLongPress();
    }
    
    // Send periodic long press hold events
    if (longPressHoldActive) {
        static unsigned long lastHoldEvent = 0;
        if (millis() - lastHoldEvent >= LONG_PRESS_HOLD_INTERVAL) {
            lastHoldEvent = millis();
            if (eventCallback) {
                eventCallback(ButtonEvent::LONG_PRESS_HOLD);
            }
        }
    }
    
    lastState = reading;
}

void Button::handleButtonPress() {
    pressStartTime = millis();
    buttonPressed = true;
    longPressHandled = false;
    longPressHoldActive = false;
    
    Serial.println("Button: Pressed");
}

void Button::handleButtonRelease() {
    unsigned long pressDuration = millis() - pressStartTime;
    
    Serial.print("Button: Released after ");
    Serial.print(pressDuration);
    Serial.println("ms");
    
    // If this was a long press that's being released, send a final LONG_PRESS event
    if (longPressHoldActive) {
        // Long press was active, now released
        Serial.println("Button: Long press released");
        if (eventCallback) {
            eventCallback(ButtonEvent::LONG_PRESS);
        }
    } else if (!longPressHandled && pressDuration < LONG_PRESS_TIME) {
        // Short press
        Serial.println("Button: Short press");
        if (eventCallback) {
            eventCallback(ButtonEvent::SHORT_PRESS);
        }
    }
    
    buttonPressed = false;
    longPressHandled = false;
    longPressHoldActive = false;
}

void Button::checkLongPress() {
    if (millis() - pressStartTime >= LONG_PRESS_TIME) {
        if (!longPressHandled) {
            longPressHandled = true;
            longPressHoldActive = true;
            
            Serial.println("Button: Long press threshold reached");
            
            // Don't send LONG_PRESS here, just start the HOLD events
            // LONG_PRESS will be sent on release
        }
    }
}

void Button::onButtonEvent(std::function<void(ButtonEvent)> callback) {
    eventCallback = callback;
}