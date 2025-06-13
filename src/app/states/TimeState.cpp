// app/states/TimeState.cpp - Updated with menu action execution
#include "TimeState.h"
#include "MenuState.h"
#include "app/Application.h"
#include "services/TimeService.h"
#include "services/NetworkService.h"
#include "animator.h"
#include "menuhandler.h"
#include <time.h>

// External reference to menu handler for checking pending actions
extern std::unique_ptr<MenuHandler> menuHandler;

TimeState::TimeState(Application* application) 
    : State(application), 
      lastUpdateTime(0), 
      colonVisible(true),
      isAnimating(false),
      lastSecond(-1),
      longPressHandled(false) {
    animator = std::make_unique<Animator>();
}

TimeState::~TimeState() {
    if (animator) {
        animator->stop();
    }
}

void TimeState::onEnter() {
    app->getDisplay()->clear();

    // Reset button handling state
    longPressHandled = false;  // Reset when entering state
    
    // Show clock icon if time is synced
    if (app->getTimeService()->isTimeSynced()) {
        app->getDisplay()->setIcon(DisplayIcon::CLOCK, true);
    }
    
    // Show WiFi icon if connected
    if (app->getNetworkService()->isConnected()) {
        app->getDisplay()->setIcon(DisplayIcon::WIFI, true);
    }
    
    isAnimating = false;
    lastSecond = -1;
}

void TimeState::onExit() {
    // Stop any ongoing animation
    if (animator) {
        animator->stop();
    }
    isAnimating = false;
}

void TimeState::onUpdate() {
    // Don't update display if ANY animation is running
    extern Animator globalAnimator;  // Get reference to global animator
    
    if (isAnimating || globalAnimator.is_running()) {
        return;  // Don't update display while any animation is active
    }
    
    unsigned long currentTime = millis();
    
    // Update every 500ms for colon blinking
    if (currentTime - lastUpdateTime >= 500) {
        lastUpdateTime = currentTime;
        updateTimeDisplay();
    }
}

void TimeState::updateTimeDisplay() {
    TimeService* timeService = app->getTimeService();
    
    if (timeService && timeService->isTimeSynced()) {
        // Get current time
        time_t now;
        tm timeinfo;
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Check if we should show date (every 20 seconds)
        if (timeinfo.tm_sec % 20 == 0 && timeinfo.tm_sec != lastSecond) {
            lastSecond = timeinfo.tm_sec;
            savedTimeInfo = timeinfo; // Save for animation callback
            showDateAnimation();
            return;
        }
        
        // Normal time display
        char buffer[10];
        strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
        
        // Update display
        app->getDisplay()->setText(buffer);
        
        // Toggle colons
        app->getDisplay()->setColon(0, colonVisible);
        app->getDisplay()->setColon(1, colonVisible);
        colonVisible = !colonVisible;
        
        // Update last second
        lastSecond = timeinfo.tm_sec;
    } else {
        // No time sync
        app->getDisplay()->setText("NO NTP");
        app->getDisplay()->setColon(0, false);
        app->getDisplay()->setColon(1, false);
    }
}

void TimeState::showDateAnimation() {
    if (isAnimating) {
        return; // Already animating
    }
    
    isAnimating = true;
    
    // Get current time for the fade out
    char timeBuffer[10];
    strftime(timeBuffer, sizeof(timeBuffer), "%H%M%S", &savedTimeInfo);
    
    // Create a copy of the saved time info that will persist
    tm* timeCopy = new tm;
    memcpy(timeCopy, &savedTimeInfo, sizeof(tm));
    
    // Start fade out animation
    animator->start_random_fade_out(timeBuffer, 100, [this, timeCopy]() {
        // After fade out, show date
        char dateBuffer[60];
        strftime(dateBuffer, sizeof(dateBuffer), "%A %d %B %Y", timeCopy);
        
        // Set the date text and run scroll animation
        animator->set_text_and_run(dateBuffer, 210, 1, [this, timeCopy]() {
            // After date scroll, fade back in with current time
            // Get the CURRENT time for the fade-in
            time_t currentNow;
            tm currentTimeinfo;
            time(&currentNow);
            localtime_r(&currentNow, &currentTimeinfo);
            
            char currentTimeBuffer[10];
            strftime(currentTimeBuffer, sizeof(currentTimeBuffer), "%H%M%S", &currentTimeinfo);
            
            animator->start_random_fade_in(currentTimeBuffer, 50, [this, timeCopy]() {
                // Animation complete
                isAnimating = false;
                delete timeCopy; // Clean up allocated memory
            });
        });
    });
}

void TimeState::onButtonEvent(ButtonEvent event) {
    Serial.print("TimeState: Button event - ");
    Serial.println(static_cast<int>(event));
    
    
    if (event == ButtonEvent::SHORT_PRESS) {
        // Reset the flag on any short press
        longPressHandled = false;
        
        // Check if there's a pending menu action to execute
        State* menuState = app->getStateManager()->getState(StateType::MENU);
        if (menuState) {
            MenuState* menu = static_cast<MenuState*>(menuState);
            
            // Check if menu has a pending selection
            if (menu->hasPendingAction()) {
                Serial.println("TimeState: Executing pending menu action");
                menu->executeSelectedAction();
                return;
            }
        }
        
        // No pending menu action - show date
        TimeService* timeService = app->getTimeService();
        if (timeService && timeService->isTimeSynced() && !isAnimating) {
            // Get current time
            time_t now;
            tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            
            // Format date
            char dateBuffer[60];
            strftime(dateBuffer, sizeof(dateBuffer), "%A %d %B %Y", &timeinfo);
            
            // Stop any animation and show date with scroll
            animator->stop();
            isAnimating = true;
            
            animator->set_text_and_run(dateBuffer, 210, 1, [this]() {
                isAnimating = false;
            });
        }
    }
    else if (event == ButtonEvent::LONG_PRESS_HOLD) {
        // Enter menu immediately on first long press hold event
        if (!longPressHandled) {
            longPressHandled = true;
            Serial.println("TimeState: Long press hold - entering menu");
            app->getStateManager()->changeState(StateType::MENU);
        }
    }
    else if (event == ButtonEvent::LONG_PRESS) {
        // Long press released - reset the flag
        Serial.println("TimeState: Long press released");
        longPressHandled = false;
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