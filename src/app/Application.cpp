// app/Application.cpp - Updated with proper NetworkService integration
#include "Application.h"
#include "hal/VfdDisplay.h"
#include "hal/Button.h"
#include "services/TimeService.h"
#include "services/NetworkService.h"
#include "services/ConfigService.h"
#include "states/TimeState.h"
//#include "states/MenuState.h"
//#include "states/TextScrollState.h"
#include "states/ConfigState.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <animator.h>

// Pin configuration
#define KEY1 D3  // Adjust this to match your button pin

// Global animator for config mode callback
extern Animator globalAnimator;
Animator globalAnimator;

// Static callback for WiFiManager
static Application* appInstance = nullptr;
static void wifiConfigModeCallback(WiFiManager *myWiFiManager) {
    if (appInstance) {
        String pwd = myWiFiManager->getConfigPortalSSID();
        Serial.print("Config mode AP: ");
        Serial.println(pwd);
        
        // Get the password from the AP name or generate one
        // The password is set in NetworkService
        // For now, we'll display "CONFIG" and the SSID
        appInstance->getDisplay()->clear();
        
        // Stop any animation
        globalAnimator.stop();
        
        // Show password with scrolling animation
        String displayPwd = myWiFiManager->getConfigPortalSSID();
        globalAnimator.set_text_and_run(displayPwd.c_str(), 255, 200);
    }
}

Application::Application() 
    : lastUpdateTime(0) {
    appInstance = this;
}

Application::~Application() {
    appInstance = nullptr;
}

bool Application::initialize() {
    Serial.println("\n=== Application::initialize() ===");
    
    // Initialize display first
    Serial.println("- Initializing display...");
    display = std::make_unique<VfdDisplay>();
    display->powerOn();
    display->setBrightness(2);
    display->setText("BOOT");
    delay(500); // Show BOOT briefly
    
    // Initialize button
    Serial.println("- Initializing button...");
    button = std::make_unique<Button>(KEY1);
    button->begin();
    button->onButtonEvent([this](ButtonEvent event) {
        this->onButtonPress(event);
    });
    
    // Initialize state manager early so we can use ConfigState
    Serial.println("- Initializing state manager...");
    stateManager = std::make_unique<StateManager>(this);
    
    // Register states
    Serial.println("- Registering states...");
    stateManager->registerState(StateType::TIME, 
        std::make_unique<TimeState>(this));
    
    // stateManager->registerState(StateType::MENU, 
    //     std::make_unique<MenuState>(this));
    
    // stateManager->registerState(StateType::TEXT_SCROLL, 
    //     std::make_unique<TextScrollState>(this));
    
    stateManager->registerState(StateType::CONFIG, 
        std::make_unique<ConfigState>(this));
    
    // Initialize network service with loading animation
    Serial.println("- Initializing network service...");
    display->setText("WiFi");
    globalAnimator.start_loading(0x01);
    
    networkService = std::make_unique<NetworkService>();
    
    // Set up network callbacks before begin()
    networkService->onConnectionChange([this](bool connected) {
        Serial.print("Network state changed: ");
        Serial.println(connected ? "Connected" : "Disconnected");
        
        stateManager->handleNetworkStateChange(connected);
        display->setIcon(DisplayIcon::WIFI, connected);
        
        if (connected) {
            // Show IP address briefly when connected
            globalAnimator.stop();
            display->setText(WiFi.localIP().toString().substring(0, 6).c_str());
            delay(2000);
        }
    });
    
    networkService->onConfigSave([this](const NetworkService::NetworkConfig& config) {
        Serial.println("Network configuration saved");
        display->setText("SAVED");
        delay(1000);
    });
    
    // Start network service
    networkService->begin();
    globalAnimator.stop();
    
    // Initialize other services
    Serial.println("- Initializing other services...");
    
    // Initialize time service after network
    timeService = std::make_unique<TimeService>();
    timeService->begin();
    
    timeService->onTimeSync([this]() {
        stateManager->handleTimeSync();
        display->setIcon(DisplayIcon::CLOCK, true);
    });
    
    configService = std::make_unique<ConfigService>();
    configService->begin();
    
    // Initialize OTA
    initializeOTA();
    
    // Show network status
    if (networkService->isConnected()) {
        display->setText(WiFi.localIP().toString().substring(0, 6).c_str());
        display->setIcon(DisplayIcon::WIFI, true);
        delay(2000);
    } else {
        display->setText("NO WiFi");
        delay(1000);
    }
    
    // Start with time display
    Serial.println("- Starting with TIME state...");
    stateManager->changeState(StateType::TIME);
    
    Serial.println("=== Application initialization complete! ===\n");
    return true;
}

void Application::update() {
    unsigned long currentTime = millis();
    
    // Update button
    button->update();
    
    // Handle OTA
    ArduinoOTA.handle();
    
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
    const char* eventName = "";
    switch (event) {
        case ButtonEvent::SHORT_PRESS:
            eventName = "SHORT_PRESS";
            break;
        case ButtonEvent::LONG_PRESS:
            eventName = "LONG_PRESS";
            break;
        case ButtonEvent::LONG_PRESS_HOLD:
            eventName = "LONG_PRESS_HOLD";
            break;
    }
    
    Serial.print("Application: Button event - ");
    Serial.println(eventName);
    
    stateManager->handleButtonEvent(event);
}

void Application::initializeOTA() {
    Serial.println("- Initializing OTA...");
    
    ArduinoOTA.setPassword("lonelybinary");
    
    ArduinoOTA.onStart([this]() {
        Serial.println("OTA: Start");
        globalAnimator.stop();
        globalAnimator.set_text_and_run("*OTA*", 210, 200);
        display->setIcon(DisplayIcon::REC, true);
    });
    
    ArduinoOTA.onEnd([this]() {
        Serial.println("OTA: End");
        globalAnimator.stop();
        display->setIcon(DisplayIcon::REC, false);
        display->setText("REBOOT");
        delay(1000);
        ESP.restart();
    });
    
    ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        static unsigned int lastPercent = 0;
        unsigned int percent = (progress / (total / 100));
        if (percent != lastPercent && percent % 10 == 0) {
            lastPercent = percent;
            char buf[7];
            sprintf(buf, "OTA%3d", percent);
            display->setText(buf);
        }
    });
    
    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.print("OTA Error: ");
        Serial.println(error);
        display->setText("OTA ERR");
        delay(2000);
    });
    
    ArduinoOTA.begin();
}

// Add getter implementations if not already present
NetworkService* Application::getNetworkService() { 
    return networkService.get(); 
}

ConfigService* Application::getConfigService() { 
    return configService.get(); 
}