// app/Application.h - Updated with OTA
#ifndef APPLICATION_H
#define APPLICATION_H

#include <memory>
#include "hal/IDisplay.h"
#include "states/StateManager.h"

// Forward declarations
class TimeService;
class NetworkService;
class ConfigService;
class IButton;

class Application {
private:
    // Core components
    std::unique_ptr<IDisplay> display;
    std::unique_ptr<IButton> button;
    std::unique_ptr<StateManager> stateManager;
    
    // Services
    std::unique_ptr<TimeService> timeService;
    std::unique_ptr<NetworkService> networkService;
    std::unique_ptr<ConfigService> configService;
    
    // Timing
    unsigned long lastUpdateTime;
    static constexpr unsigned long UPDATE_INTERVAL = 100; // 100ms
    
    // Private methods
    void initializeOTA();
    
public:
    Application();
    ~Application();
    
    // Lifecycle
    bool initialize();
    void update();
    
    // Getters for states to access services
    IDisplay* getDisplay() { return display.get(); }
    TimeService* getTimeService() { return timeService.get(); }
    NetworkService* getNetworkService();
    ConfigService* getConfigService();
    StateManager* getStateManager() { return stateManager.get(); }
    
    // Event handlers
    void onButtonPress(ButtonEvent event);
};

#endif // APPLICATION_H