#include <Arduino.h>
#include "app/Application.h"

// Global application instance
std::unique_ptr<Application> app;

void setup() {
    Serial.begin(115200);
    Serial.println("\n\nVFD Clock v2.0 Starting...");
    
    // Create and initialize application
    app = std::make_unique<Application>();
    
    if (!app->initialize()) {
        Serial.println("Failed to initialize application!");
        while (1) {
            delay(1000);
        }
    }
    
    Serial.println("Application initialized successfully");
}

void loop() {
    app->update();
    
    // Handle any other Arduino-specific tasks
    yield();
}