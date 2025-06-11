// app/states/MenuState.cpp - Updated with Animator
#include "MenuState.h"
#include "app/Application.h"
#include "menuhandler.h"
#include "animator.h"
#include <Arduino.h>

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
    
    // Load menu items
    menuHandler->initializeMenuItems();
    
    // Set up special action callback
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
            // Start WiFi config portal
            app->getNetworkService()->resetSettings();
            ESP.restart();
        }
        else if (strcmp(item, "ai") == 0) {
            // Will implement AI state later
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
    
    // Start scrolling
    isScrolling = true;
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
    if (event == ButtonEvent::SHORT_PRESS) {
        // Short press: select current item
        Serial.println("MenuState: Selecting current item");
        selectCurrentItem();
    }
    else if (event == ButtonEvent::LONG_PRESS_HOLD) {
        // Continue scrolling
        isScrolling = true;
    }
    else if (event == ButtonEvent::LONG_PRESS) {
        // Stop scrolling and flash selection
        isScrolling = false;
        flashMenuItem();
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
        app->getDisplay()->setText(items[index].menu.c_str());
    }
}

void MenuState::scrollToNext() {
    String nextItem = menuHandler->scrollToNextItem();
    app->getDisplay()->setText(nextItem.c_str());
}

void MenuState::selectCurrentItem() {
    // Get the selected text from menu handler
    String selectedText = menuHandler->selectCurrentItem();
    
    if (selectedText.length() > 0) {
        // Use global animator to show the selected text
        Serial.print("MenuState: Selected text - ");
        Serial.println(selectedText);
        
        // Show play icon and animate text
        app->getDisplay()->setIcon(DisplayIcon::PLAY, true);
        globalAnimator.set_text_and_run(selectedText.c_str(), 210, 1, [this]() {
            // After animation, return to time display
            app->getDisplay()->setIcon(DisplayIcon::PLAY, false);
            app->getStateManager()->changeState(StateType::TIME);
        });
    } else {
        // Special action was handled, or empty selection
        // State change is handled by the special action callback
    }
}

void MenuState::flashMenuItem() {
    // Visual feedback - blink current menu text
    Serial.println("MenuState: Flashing selection");
    String currentText = menuHandler->getMenuItems()[menuHandler->getCurrentMenuIndex()].menu;
    
    for (int i = 0; i < 3; i++) {
        app->getDisplay()->clear();
        delay(150);
        app->getDisplay()->setText(currentText.c_str());
        delay(150);
    }
}

void MenuState::startFadeDemo() {
    // Chain multiple animations together
    globalAnimator.start_random_fade_in("FADE", 100, [this]() {
        globalAnimator.start_random_fade_in("DEMO", 50, [this]() {
            globalAnimator.start_typewriter_effect("TYPE", 200, [this]() {
                globalAnimator.start_reveal_effect("REVEAL", 150, [this]() {
                    globalAnimator.start_wave_effect("WAVE EFFECT", 100, [this]() {
                        globalAnimator.start_fade_out("DONE", 120, [this]() {
                            // Return to time after demo
                            app->getStateManager()->changeState(StateType::TIME);
                        });
                    });
                });
            }, 300);
        }, 500);
    }, 500);
}