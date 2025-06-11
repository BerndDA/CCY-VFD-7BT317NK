// app/states/MenuState.cpp - Fixed includes
#include "MenuState.h"
#include "app/Application.h"
#include "services/NetworkService.h"  // Add this include
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
            app->getStateManager()->changeState(StateType::TIME);
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
    
    // Don't start scrolling immediately - wait for long press hold
    isScrolling = false;
    lastScrollTime = millis();
}

void MenuState::onExit() {
    Serial.println("MenuState: Exiting menu");
    isScrolling = false;
    globalAnimator.stop(); // Stop any ongoing animation
}

void MenuState::onUpdate() {
    // Auto-scroll through menu items while in long-press mode
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
        // Short press: select current item
        Serial.println("MenuState: Selecting current item");
        selectCurrentItem();
    }
    else if (event == ButtonEvent::LONG_PRESS) {
        // Long press detected - start scrolling
        Serial.println("MenuState: Long press - starting scroll");
        isScrolling = true;
        lastScrollTime = millis();
    }
    else if (event == ButtonEvent::LONG_PRESS_HOLD) {
        // Continue scrolling while button is held
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

void MenuState::selectCurrentItem() {
    // Get the selected text from menu handler
    String selectedText = menuHandler->selectCurrentItem();
    
    if (selectedText.length() > 0) {
        // We got text to display
        Serial.print("MenuState: Selected text - ");
        Serial.println(selectedText);
        
        // Stop any ongoing animation
        globalAnimator.stop();
        
        // Show play icon and animate text
        app->getDisplay()->setIcon(DisplayIcon::PLAY, true);
        
        // Set up animation end callback
        // globalAnimator.onEnd([this]() {
        //     // After animation, clear play icon and return to time
        //     app->getDisplay()->setIcon(DisplayIcon::PLAY, false);
        //     app->getStateManager()->changeState(StateType::TIME);
        // });
        
        // Start the text animation
        globalAnimator.set_text_and_run(selectedText.c_str(), 210);
    } else {
        // Special action was handled by callback, or empty selection
        // The special action callback handles state changes if needed
        Serial.println("MenuState: Special action or empty selection");
    }
}

void MenuState::flashMenuItem() {
    // Visual feedback - blink current menu text when button is released after long press
    Serial.println("MenuState: Flashing selection");
    isScrolling = false; // Stop scrolling
    
    const auto& items = menuHandler->getMenuItems();
    uint8_t index = menuHandler->getCurrentMenuIndex();
    
    if (index < items.size()) {
        String currentText = items[index].menu;
        
        // Flash the selected item
        for (int i = 0; i < 4; i++) {
            app->getDisplay()->clear();
            delay(100);
            app->getDisplay()->setText(currentText.c_str());
            delay(100);
        }
    }
}

void MenuState::startFadeDemo() {
    // Animation demo sequence
    Serial.println("MenuState: Starting animation demo");
    
    // Chain multiple animations together with callbacks
    globalAnimator.start_random_fade_in("RANDOM", 100, [this]() {
        globalAnimator.start_random_fade_in("FADE", 50, [this]() {
            globalAnimator.start_typewriter_effect("TYPE", 200, [this]() {
                globalAnimator.start_reveal_effect("REVEAL", 150, [this]() {
                    globalAnimator.start_wave_effect("WAVE EFFECT DEMO", 100, [this]() {
                        globalAnimator.start_fade_out("DONE", 120, [this]() {
                            // Return to time after demo
                            app->getStateManager()->changeState(StateType::TIME);
                        }, 500);
                    });
                });
            }, 300);
        }, 500);
    }, 500);
}