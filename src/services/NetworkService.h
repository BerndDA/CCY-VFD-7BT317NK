// services/NetworkService.h
#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include <functional>
#include <ESP8266WiFi.h>

class NetworkService {
private:
    bool connected;
    std::function<void(bool)> onConnectionChangeCallback;
    
public:
    NetworkService() : connected(false) {}
    
    void begin() {
        // For now, just check if already connected
        connected = WiFi.isConnected();
    }
    
    void update() {
        bool currentState = WiFi.isConnected();
        if (currentState != connected) {
            connected = currentState;
            if (onConnectionChangeCallback) {
                onConnectionChangeCallback(connected);
            }
        }
    }
    
    bool isConnected() const { return connected; }
    
    void onConnectionChange(std::function<void(bool)> callback) {
        onConnectionChangeCallback = callback;
    }
};
#endif // NETWORK_SERVICE_H