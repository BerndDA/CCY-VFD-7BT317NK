// app/Application.cpp
#include "Application.h"
#include "hal/VfdDisplay.h"
#include "hal/Button.h"
#include "services/TimeService.h"
#include "services/NetworkService.h"
#include "services/ConfigService.h"
#include "states/TimeState.h"
#include <Arduino.h>

// Assuming KEY1 is defined in your config
#define KEY1 D3  // Adjust this to match your button pin

Application::Application() 
    : lastUpdateTime(0) {
}

Application::~Application() {
    // Cleanup handled by smart pointers
}

bool Application::initialize() {
    Serial.println("Initializing application components...");
    
    // Initialize display
    Serial.println("- Initializing display...");
    display = std::make_unique<VfdDisplay>();
    display->powerOn();
    display->setBrightness(2);
    display->setText("BOOT");
    
    // Initialize button
    Serial.println("- Initializing button...");
    button = std::make_unique<Button>(KEY1);
    button->begin();
    button->onButtonEvent([this](ButtonEvent event) {
        this->onButtonPress(event);
    });
    
    // Initialize services
    Serial.println("- Initializing services...");
    timeService = std::make_unique<TimeService>();
    timeService->begin();
    
    networkService = std::make_unique<NetworkService>();
    networkService->begin();
    
    configService = std::make_unique<ConfigService>();
    configService->begin();
    
    // Initialize state manager
    Serial.println("- Initializing state manager...");
    stateManager = std::make_unique<StateManager>(this);
    
    // Register states
    Serial.println("- Registering states...");
    stateManager->registerState(StateType::TIME, 
        std::make_unique<TimeState>(this));
    
    // TODO: Register other states as you implement them
    // stateManager->registerState(StateType::MENU, 
    //     std::make_unique<MenuState>(this));
    
    // Set up service callbacks
    timeService->onTimeSync([this]() {
        stateManager->handleTimeSync();
        display->setIcon(DisplayIcon::CLOCK, true);
    });
    
    networkService->onConnectionChange([this](bool connected) {
        stateManager->handleNetworkStateChange(connected);
        display->setIcon(DisplayIcon::WIFI, connected);
    });
    
    // Start with time display
    Serial.println("- Starting with TIME state...");
    stateManager->changeState(StateType::TIME);
    
    Serial.println("Application initialization complete!");
    return true;
}

void Application::update() {
    unsigned long currentTime = millis();
    
    // Update button
    button->update();
    
    // Update at fixed interval
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
        lastUpdateTime = currentTime;
        
        // Update current state
        stateManager->update();
        
        // Update services
        if (timeService) {
            timeService->update();
        }
        
        if (networkService) {
            networkService->update();
        }
    }
}

void Application::onButtonPress(ButtonEvent event) {
    Serial.print("Button event: ");
    Serial.println(static_cast<int>(event));
    stateManager->handleButtonEvent(event);
}

// Add getters implementation
NetworkService* Application::getNetworkService() { 
    return networkService.get(); 
}

ConfigService* Application::getConfigService() { 
    return configService.get(); 
}