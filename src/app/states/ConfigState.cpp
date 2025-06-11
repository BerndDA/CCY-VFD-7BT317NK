// app/states/ConfigState.cpp
#include "ConfigState.h"
#include "app/Application.h"
#include "services/NetworkService.h"
#include <Arduino.h>

ConfigState::ConfigState(Application* application) 
    : State(application), lastUpdateTime(0), scrollPosition(0) {
}

void ConfigState::onEnter() {
    Serial.println("ConfigState: Entering config mode");
    Serial.print("ConfigState: Password: ");
    Serial.println(password.c_str());
    
    // Clear display and show password
    app->getDisplay()->clear();
    
    // Flash WiFi icon
    app->getDisplay()->setIcon(DisplayIcon::WIFI, true);
    
    scrollPosition = 0;
    lastUpdateTime = millis();
}

void ConfigState::onExit() {
    Serial.println("ConfigState: Exiting config mode");
}

void ConfigState::onUpdate() {
    unsigned long currentTime = millis();
    
    // Scroll the password every 300ms if it's longer than 6 chars
    if (currentTime - lastUpdateTime >= 300) {
        lastUpdateTime = currentTime;
        
        if (password.length() > 6) {
            // Create a scrolling display of the password
            std::string displayText = password + "   "; // Add spaces for gap
            
            // Calculate the visible portion
            std::string visible = displayText.substr(scrollPosition, 6);
            
            // If we need more characters, wrap around
            if (visible.length() < 6) {
                visible += displayText.substr(0, 6 - visible.length());
            }
            
            app->getDisplay()->setText(visible.c_str());
            
            // Update scroll position
            scrollPosition++;
            if (scrollPosition >= displayText.length()) {
                scrollPosition = 0;
            }
        } else {
            // Password fits on display
            app->getDisplay()->setText(password.c_str());
        }
        
        // Toggle WiFi icon for visual feedback
        static bool wifiIconState = true;
        wifiIconState = !wifiIconState;
        app->getDisplay()->setIcon(DisplayIcon::WIFI, wifiIconState);
    }
}

void ConfigState::onButtonEvent(ButtonEvent event) {
    if (event == ButtonEvent::SHORT_PRESS) {
        // Short press: Show "CONFIG" briefly
        app->getDisplay()->setText("CONFIG");
        delay(1000);
    }
    else if (event == ButtonEvent::LONG_PRESS) {
        // Long press: Exit config mode (would need to implement cancel in NetworkService)
        Serial.println("ConfigState: Long press - exiting config");
        app->getStateManager()->changeState(StateType::TIME);
    }
}