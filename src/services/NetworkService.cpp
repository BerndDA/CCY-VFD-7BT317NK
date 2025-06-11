// services/NetworkService.cpp
#include "NetworkService.h"
#include <EEPROM.h>
#include <Arduino.h>

// Static member for callback
static NetworkService* networkServiceInstance = nullptr;
static String generatedPassword;

NetworkService::NetworkService() 
    : connected(false), 
      configMode(false),
      lastConnectionCheck(0) {
    networkServiceInstance = this;
}

NetworkService::~NetworkService() {
    networkServiceInstance = nullptr;
    EEPROM.end();
}

void NetworkService::begin() {
    Serial.println("NetworkService: Initializing...");
    
    // Initialize EEPROM
    EEPROM.begin(sizeof(NetworkConfig));
    
    // Load saved configuration
    loadConfig();
    
    // Create WiFiManager instance
    wifiManager = std::make_unique<WiFiManager>();
    
    // Set up custom parameters
    setupWiFiManager();
    
    // Set config save callback
    wifiManager->setSaveConfigCallback([this]() {
        this->handleConfigSave();
    });
    
    // Set AP callback
    wifiManager->setAPCallback(NetworkService::configModeCallback);
    
    // Try to connect with saved credentials
    // Generate a unique password for AP mode
    generatedPassword = String(random(10000000, 99999999));
    generatedPassword.toUpperCase();
    
    Serial.println("NetworkService: Attempting to connect to WiFi...");
    
    // Set shorter timeout for faster startup
    wifiManager->setConfigPortalTimeout(180); // 3 minutes
    
    // Try to connect
    bool success = wifiManager->autoConnect("VFD-03", generatedPassword.c_str());
    
    if (success) {
        connected = true;
        Serial.print("NetworkService: Connected to WiFi! IP: ");
        Serial.println(WiFi.localIP());
        
        // Notify callback
        if (onConnectionChangeCallback) {
            onConnectionChangeCallback(true);
        }
    } else {
        Serial.println("NetworkService: Failed to connect to WiFi");
        connected = false;
    }
    
    configMode = false;
}

void NetworkService::update() {
    unsigned long currentTime = millis();
    
    // Periodically check connection status
    if (currentTime - lastConnectionCheck >= CONNECTION_CHECK_INTERVAL) {
        lastConnectionCheck = currentTime;
        
        bool currentState = WiFi.isConnected();
        if (currentState != connected) {
            connected = currentState;
            
            Serial.print("NetworkService: Connection state changed to ");
            Serial.println(connected ? "connected" : "disconnected");
            
            // If disconnected, try to reconnect
            if (!connected && !configMode) {
                Serial.println("NetworkService: Attempting to reconnect...");
                WiFi.reconnect();
            }
            
            if (onConnectionChangeCallback) {
                onConnectionChangeCallback(connected);
            }
        }
    }
}

void NetworkService::startConfigPortal() {
    Serial.println("NetworkService: Starting config portal...");
    configMode = true;
    
    // Generate new password
    generatedPassword = String(random(10000000, 99999999));
    generatedPassword.toUpperCase();
    
    // Start config portal
    wifiManager->startConfigPortal("VFD-03", generatedPassword.c_str());
    
    configMode = false;
    
    // Check if we're now connected
    connected = WiFi.isConnected();
    if (onConnectionChangeCallback) {
        onConnectionChangeCallback(connected);
    }
}

void NetworkService::resetSettings() {
    Serial.println("NetworkService: Resetting WiFi settings...");
    wifiManager->resetSettings();
    
    // Clear our config too
    memset(&config, 0, sizeof(config));
    saveConfig();
}

void NetworkService::loadConfig() {
    Serial.println("NetworkService: Loading configuration...");
    EEPROM.get(0, config);
    
    // Validate loaded config (check for reasonable values)
    // If first byte is 0xFF, EEPROM is uninitialized
    if (config.mqtt_server[0] == 0xFF) {
        Serial.println("NetworkService: No saved config, using defaults");
        memset(&config, 0, sizeof(config));
        strcpy(config.mqtt_port, "1883"); // Default MQTT port
    } else {
        Serial.println("NetworkService: Loaded saved configuration");
        Serial.print("  MQTT Server: ");
        Serial.println(config.mqtt_server);
        Serial.print("  MQTT Port: ");
        Serial.println(config.mqtt_port);
        Serial.println("  API Key: [configured]");
        Serial.println("  Assistant ID: [configured]");
    }
}

void NetworkService::saveConfig() {
    Serial.println("NetworkService: Saving configuration...");
    EEPROM.put(0, config);
    EEPROM.commit();
}

void NetworkService::setupWiFiManager() {
    // Create custom parameters
    custom_mqtt_server = std::make_unique<WiFiManagerParameter>(
        "server", "MQTT Server", config.mqtt_server, 40);
    
    custom_mqtt_port = std::make_unique<WiFiManagerParameter>(
        "port", "MQTT Port", config.mqtt_port, 6);
    
    custom_apikey = std::make_unique<WiFiManagerParameter>(
        "apikey", "OpenAI API Key", config.api_key, 180);
    
    custom_assistantid = std::make_unique<WiFiManagerParameter>(
        "assistant", "Assistant ID", config.assistant_id, 40);
    
    // Add parameters to WiFiManager
    wifiManager->addParameter(custom_mqtt_server.get());
    wifiManager->addParameter(custom_mqtt_port.get());
    wifiManager->addParameter(custom_apikey.get());
    wifiManager->addParameter(custom_assistantid.get());
}

void NetworkService::handleConfigSave() {
    Serial.println("NetworkService: Configuration save callback triggered");
    
    // Copy values from parameters to config
    strcpy(config.mqtt_server, custom_mqtt_server->getValue());
    strcpy(config.mqtt_port, custom_mqtt_port->getValue());
    strcpy(config.api_key, custom_apikey->getValue());
    strcpy(config.assistant_id, custom_assistantid->getValue());
    
    // Save to EEPROM
    saveConfig();
    
    // Notify callback
    if (onConfigSaveCallback) {
        onConfigSaveCallback(config);
    }
}

void NetworkService::configModeCallback(WiFiManager *myWiFiManager) {
    Serial.println("NetworkService: Entered config mode");
    Serial.print("Config AP: ");
    Serial.println(myWiFiManager->getConfigPortalSSID());
    Serial.print("Password: ");
    Serial.println(generatedPassword);
    
    // If we have access to the display through the instance
    if (networkServiceInstance) {
        networkServiceInstance->configMode = true;
    }
}