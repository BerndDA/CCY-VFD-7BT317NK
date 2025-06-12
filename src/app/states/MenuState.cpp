// app/states/MenuState.cpp - Simplified implementation
#include "MenuState.h"
#include "app/Application.h"
#include "services/NetworkService.h"
#include "menuhandler.h"
#include "animator.h"
#include <Arduino.h>
#include <LittleFS.h>

// Global animator for menu animations
extern Animator globalAnimator;

MenuState::MenuState(Application* application) 
    : State(application), lastScrollTime(0), isScrolling(false) {
    menuHandler = std::make_unique<MenuHandler>();
}

MenuState::~MenuState() = default;

void MenuState::onEnter() {
    Serial.println("MenuState: Entering menu");
    
    // Initialize menu handler if needed
    if (!menuHandler->begin()) {
        Serial.println("MenuState: Failed to initialize menu handler");
        app->getDisplay()->setText("NO MENU");
        return;
    }
    
    // Load menu items from data.json
    menuHandler->initializeMenuItems();
    
    // Check if menu items were loaded
    if (menuHandler->getMenuItems().empty()) {
        Serial.println("MenuState: No menu items loaded");
        app->getDisplay()->setText("EMPTY");
        delay(1000);
        app->getStateManager()->changeState(StateType::TIME);
        return;
    }
    
    // Set up special action callback for non-file menu items
    menuHandler->setSpecialActionCallback([this](const char* item) {
        Serial.print("MenuState: Special action - ");
        Serial.println(item);
        
        if (strcmp(item, "update") == 0) {
            app->getDisplay()->setText("UPDATE");
            delay(1000);
            // Clear all files in LittleFS
            Dir dir = LittleFS.openDir("/");
            while (dir.next()) {
                String fileName = dir.fileName();
                Serial.print("Deleting file: ");
                Serial.println(fileName);
                LittleFS.remove(fileName);
            }
            ESP.restart();
        }
        else if (strcmp(item, "config") == 0) {
            app->getDisplay()->setText("CONFIG");
            delay(1000);
            // Reset WiFi settings and restart
            app->getNetworkService()->resetSettings();
            ESP.restart();
        }
        else if (strcmp(item, "ai") == 0) {
            // AI menu item - will implement AI state later
            app->getDisplay()->setText("AI");
            delay(1000);
        }
        else if (strcmp(item, "demo") == 0) {
            // Start animation demo
            Serial.println("Starting fade demo...");
            globalAnimator.stop();
            startFadeDemo();
        }
    });
    
    // Display first menu item
    displayCurrentMenuItem();
    
    // Start scrolling immediately when entering menu via long press
    isScrolling = true;
    lastScrollTime = millis();
}

void MenuState::onExit() {
    Serial.println("MenuState: Exiting menu");
    isScrolling = false;
    globalAnimator.stop(); // Stop any ongoing animation
}

void MenuState::onUpdate() {
    // Auto-scroll through menu items while button is held
    if (isScrolling) {
        unsigned long currentTime = millis();
        if (currentTime - lastScrollTime >= SCROLL_INTERVAL) {
            lastScrollTime = currentTime;
            scrollToNext();
        }
    }
}

void MenuState::onButtonEvent(ButtonEvent event) {
    Serial.print("MenuState: Button event - ");
    Serial.println(static_cast<int>(event));
    
    if (event == ButtonEvent::SHORT_PRESS) {
        // Short press in menu should not do anything
        Serial.println("MenuState: Ignoring short press in menu");
    }
    else if (event == ButtonEvent::LONG_PRESS) {
        // Long press released - stop scrolling and flash selected item
        Serial.println("MenuState: Long press released - flashing and saving selection");
        isScrolling = false;
        
        // Save the selected index as pending
        menuHandler->pendingMenuIndex = menuHandler->getCurrentMenuIndex();
        Serial.print("MenuState: Saved pending index: ");
        Serial.println(menuHandler->pendingMenuIndex);
        
        // Flash the selected item
        flashMenuItem();
        
        // After flashing, return to time display
        app->getStateManager()->changeState(StateType::TIME);
    }
    else if (event == ButtonEvent::LONG_PRESS_HOLD) {
        // Continue scrolling while button is held
        // This keeps the scrolling active
        isScrolling = true;
    }
}

void MenuState::displayCurrentMenuItem() {
    const auto& items = menuHandler->getMenuItems();
    if (items.empty()) {
        app->getDisplay()->setText("EMPTY");
        return;
    }
    
    uint8_t index = menuHandler->getCurrentMenuIndex();
    if (index < items.size()) {
        Serial.print("MenuState: Displaying item ");
        Serial.print(index);
        Serial.print(": ");
        Serial.println(items[index].menu);
        app->getDisplay()->setText(items[index].menu.c_str());
    }
}

void MenuState::scrollToNext() {
    String nextItem = menuHandler->scrollToNextItem();
    Serial.print("MenuState: Scrolled to: ");
    Serial.println(nextItem);
    app->getDisplay()->setText(nextItem.c_str());
}

void MenuState::flashMenuItem() {
    // Visual feedback - blink current menu text
    Serial.println("MenuState: Flashing selection");
    
    const auto& items = menuHandler->getMenuItems();
    uint8_t index = menuHandler->getCurrentMenuIndex();
    
    if (index < items.size()) {
        String currentText = items[index].menu;
        
        // Flash the selected item 4 times
        for (int i = 0; i < 4; i++) {
            app->getDisplay()->clear();
            delay(100);
            app->getDisplay()->setText(currentText.c_str());
            delay(100);
        }
    }
}

bool MenuState::hasPendingAction() const {
    return menuHandler && menuHandler->hasPendingAction();
}

void MenuState::executeSelectedAction() {
    if (!hasPendingAction()) {
        return;
    }
    
    Serial.print("MenuState: Executing action for pending index ");
    Serial.println(menuHandler->pendingMenuIndex);
    
    // Set current index to the pending selection
    menuHandler->setCurrentMenuIndex(menuHandler->pendingMenuIndex);
    
    // Execute the action
    String result = menuHandler->selectCurrentItem();
    
    if (result.length() > 0) {
        // Text to display - use animator
        globalAnimator.stop();
        app->getDisplay()->setIcon(DisplayIcon::PLAY, true);
        
        globalAnimator.onEnd([]() {
            // The global animator's onEnd is already set up in the original code
            // to clear the PLAY icon and return to the appropriate state
        });
        
        globalAnimator.set_text_and_run(result.c_str(), 210);
    }
    
    // Clear the pending action
    menuHandler->clearPendingAction();
}

void MenuState::startFadeDemo() {
    // Animation demo sequence
    Serial.println("MenuState: Starting animation demo");
    
    // Set callback to return to time after demo
    globalAnimator.onEnd([]() {
        // Note: We can't access 'this' in a plain function pointer
        // The animation will end and we'll handle state change differently
    });
    
    // Chain multiple animations together
    // First animation: Random fade-in effect (fast)
    globalAnimator.start_random_fade_in("RANDOM", 100, []() {
        // Second animation: Random fade-in effect (slower)
        globalAnimator.start_random_fade_in("FADE", 50, []() {
            // Third animation: Typewriter effect
            globalAnimator.start_typewriter_effect("TYPE", 200, []() {
                // Fourth animation: Reveal effect
                globalAnimator.start_reveal_effect("REVEAL", 150, []() {
                    // Fifth animation: Wave effect
                    globalAnimator.start_wave_effect("WAVE", 100, []() {
                        // Final animation: Fade out
                        globalAnimator.start_fade_out("DONE", 120);
                    });
                });
            }, 300);
        }, 500);
    }, 500);
}