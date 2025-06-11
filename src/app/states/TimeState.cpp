// states/TimeState.cpp
#include "TimeState.h"
#include "app/Application.h"
#include "services/TimeService.h"
#include "services/NetworkService.h"
#include <time.h>

TimeState::TimeState(Application* application) 
    : State(application), lastUpdateTime(0), colonVisible(true) {
}

void TimeState::onEnter() {
    app->getDisplay()->clear();
    
    // Show clock icon if time is synced
    if (app->getTimeService()->isTimeSynced()) {
        app->getDisplay()->setIcon(DisplayIcon::CLOCK, true);
    }
    
    // Show WiFi icon if connected
    if (app->getNetworkService()->isConnected()) {
        app->getDisplay()->setIcon(DisplayIcon::WIFI, true);
    }
}

void TimeState::onExit() {
    // Don't clear icons - they should persist across states
}

void TimeState::onUpdate() {
    unsigned long currentTime = millis();
    
    // Update every 500ms
    if (currentTime - lastUpdateTime >= 500) {
        lastUpdateTime = currentTime;
        
        TimeService* timeService = app->getTimeService();
        if (timeService && timeService->isTimeSynced()) {
            // Get current time
            time_t now;
            tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            
            // Format time string
            char buffer[10];
            strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
            
            // Update display
            app->getDisplay()->setText(buffer);
            
            // Toggle colons
            app->getDisplay()->setColon(0, colonVisible);
            app->getDisplay()->setColon(1, colonVisible);
            colonVisible = !colonVisible;
        } else {
            app->getDisplay()->setText("NO NTP");
            app->getDisplay()->setColon(0, false);
            app->getDisplay()->setColon(1, false);
        }
    }
}

void TimeState::onButtonEvent(ButtonEvent event) {
    if (event == ButtonEvent::SHORT_PRESS) {
        // Short press: Show date temporarily
        TimeService* timeService = app->getTimeService();
        if (timeService && timeService->isTimeSynced()) {
            String date = timeService->getFormattedDate();
            Serial.print("Date: ");
            Serial.println(date);
            // In the future, you could trigger a scroll animation here
            // For now, just display first 6 chars
            app->getDisplay()->setText(date.substring(0, 6).c_str());
        }
    } else if (event == ButtonEvent::LONG_PRESS) {
        // Long press: Enter menu
        Serial.println("Long press detected - would enter menu");
        // app->getStateManager()->changeState(StateType::MENU);
        // For now, just flash the display
        app->getDisplay()->clear();
        delay(200);
        onUpdate(); // Force immediate update
    }
}

void TimeState::onNetworkStateChange(bool connected) {
    // Network state changed - update WiFi icon
    app->getDisplay()->setIcon(DisplayIcon::WIFI, connected);
    
    // If just connected, trigger time sync
    if (connected && !app->getTimeService()->isTimeSynced()) {
        app->getTimeService()->syncTime();
    }
}

void TimeState::onTimeSync() {
    // Time was synchronized - show clock icon
    app->getDisplay()->setIcon(DisplayIcon::CLOCK, true);
    Serial.println("TimeState: Time synchronized!");
}