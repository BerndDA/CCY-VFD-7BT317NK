// services/NetworkService.h
#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <functional>
#include <memory>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

class NetworkService {
public:
    struct NetworkConfig {
        char mqtt_server[40];
        char mqtt_port[6];
        char api_key[180];
        char assistant_id[40];
    };

private:
    bool connected;
    bool configMode;
    std::function<void(bool)> onConnectionChangeCallback;
    std::function<void(const NetworkConfig&)> onConfigSaveCallback;
    
    std::unique_ptr<WiFiManager> wifiManager;
    NetworkConfig config;
    
    // WiFiManager parameters
    std::unique_ptr<WiFiManagerParameter> custom_mqtt_server;
    std::unique_ptr<WiFiManagerParameter> custom_mqtt_port;
    std::unique_ptr<WiFiManagerParameter> custom_apikey;
    std::unique_ptr<WiFiManagerParameter> custom_assistantid;
    
    // Connection management
    unsigned long lastConnectionCheck;
    static constexpr unsigned long CONNECTION_CHECK_INTERVAL = 5000; // Check every 5 seconds
    
public:
    NetworkService();
    ~NetworkService();
    
    void begin();
    void update();
    
    bool isConnected() const { return connected; }
    bool isInConfigMode() const { return configMode; }
    const NetworkConfig& getConfig() const { return config; }
    
    // Force configuration portal
    void startConfigPortal();
    
    // Reset WiFi settings
    void resetSettings();
    
    // Callbacks
    void onConnectionChange(std::function<void(bool)> callback) {
        onConnectionChangeCallback = callback;
    }
    
    void onConfigSave(std::function<void(const NetworkConfig&)> callback) {
        onConfigSaveCallback = callback;
    }
    
private:
    void loadConfig();
    void saveConfig();
    void setupWiFiManager();
    void handleConfigSave();
    static void configModeCallback(WiFiManager *myWiFiManager);
};

#endif // NETWORK_SERVICE_H