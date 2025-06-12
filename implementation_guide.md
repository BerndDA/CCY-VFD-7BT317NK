# Getting Started with Layer-Based Architecture Implementation

## Step 1: Create the Directory Structure

First, create the new directory structure alongside your existing code:

```bash
src/
├── main_new.cpp          # New main file (rename to main.cpp later)
├── app/
│   ├── Application.h
│   ├── Application.cpp
│   └── states/
│       ├── State.h
│       └── StateManager.h
│       └── StateManager.cpp
├── hal/
│   ├── IDisplay.h
│   ├── VfdDisplay.h
│   ├── VfdDisplay.cpp
│   ├── IButton.h
│   └── Button.h
│   └── Button.cpp
├── services/
│   ├── TimeService.h
│   └── TimeService.cpp
└── config/
    └── Config.h
```

## Step 2: Define Core Interfaces

### 2.1 Create the Display Interface (hal/IDisplay.h)

```cpp
#ifndef IDISPLAY_H
#define IDISPLAY_H

#include <string>
#include <cstdint>

// Display icons enumeration
enum class DisplayIcon : uint32_t {
    NONE = 0x000000,
    REC = 0x000800,
    CLOCK = 0x000400,
    CUBE_3D = 0x000200,
    WIFI = 0x000100,
    PLAY = 0x00A015,
    G1_ALL = 0xFFFF1F
};

class IDisplay {
public:
    virtual ~IDisplay() = default;
    
    // Text operations
    virtual void setText(const std::string& text) = 0;
    virtual void setCharAt(size_t index, char c) = 0;
    virtual void clear() = 0;
    
    // Icon operations
    virtual void setIcon(DisplayIcon icon, bool enabled) = 0;
    virtual void clearIcons() = 0;
    
    // Display properties
    virtual void setBrightness(uint8_t level) = 0;
    virtual void setColon(uint8_t colonNumber, bool enabled) = 0;
    
    // Power management
    virtual void powerOn() = 0;
    virtual void powerOff() = 0;
};

#endif // IDISPLAY_H
```

### 2.2 Create the State Interface (app/states/State.h)

```cpp
#ifndef STATE_H
#define STATE_H

#include <memory>

// Forward declarations
class Application;
class StateManager;

enum class StateType {
    TIME,
    MENU,
    TEXT_SCROLL,
    AI_CHAT,
    CONFIG,
    NO_TIME
};

enum class ButtonEvent {
    SHORT_PRESS,
    LONG_PRESS,
    LONG_PRESS_HOLD
};

class State {
protected:
    Application* app;
    
public:
    State(Application* application) : app(application) {}
    virtual ~State() = default;
    
    // Lifecycle methods
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void onUpdate() = 0;
    
    // Event handlers
    virtual void onButtonEvent(ButtonEvent event) = 0;
    virtual void onNetworkStateChange(bool connected) {}
    virtual void onTimeSync() {}
    
    // State identification
    virtual StateType getType() const = 0;
    virtual const char* getName() const = 0;
};

#endif // STATE_H
```

### 2.3 Create the Application Class (app/Application.h)

```cpp
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
    
public:
    Application();
    ~Application();
    
    // Lifecycle
    bool initialize();
    void update();
    
    // Getters for states to access services
    IDisplay* getDisplay() { return display.get(); }
    TimeService* getTimeService() { return timeService.get(); }
    StateManager* getStateManager() { return stateManager.get(); }
    
    // Event handlers
    void onButtonPress(ButtonEvent event);
};

#endif // APPLICATION_H
```

## Step 3: Implement the VFD Display Wrapper

### 3.1 VfdDisplay.h

```cpp
#ifndef VFD_DISPLAY_H
#define VFD_DISPLAY_H

#include "hal/IDisplay.h"
#include <bitset>

class VfdDisplay : public IDisplay {
private:
    uint8_t brightness;
    std::bitset<32> activeIcons;
    bool colonStates[2];
    bool powered;
    
    // Helper methods
    void updateDisplay();
    uint32_t convertIconEnum(DisplayIcon icon);
    
public:
    VfdDisplay();
    ~VfdDisplay() override;
    
    // IDisplay implementation
    void setText(const std::string& text) override;
    void setCharAt(size_t index, char c) override;
    void clear() override;
    
    void setIcon(DisplayIcon icon, bool enabled) override;
    void clearIcons() override;
    
    void setBrightness(uint8_t level) override;
    void setColon(uint8_t colonNumber, bool enabled) override;
    
    void powerOn() override;
    void powerOff() override;
};

#endif // VFD_DISPLAY_H
```

### 3.2 VfdDisplay.cpp

```cpp
#include "VfdDisplay.h"
#include "gui.h" // Your existing GUI functions

VfdDisplay::VfdDisplay() 
    : brightness(2), activeIcons(0), powered(true) {
    colonStates[0] = false;
    colonStates[1] = false;
}

VfdDisplay::~VfdDisplay() {
    powerOff();
}

void VfdDisplay::setText(const std::string& text) {
    if (!powered) return;
    
    // Use existing vfd_gui_set_text
    vfd_gui_set_text(text.c_str());
}

void VfdDisplay::setCharAt(size_t index, char c) {
    if (!powered || index >= 6) return;
    
    // Use existing vfd_gui_set_one_text
    vfd_gui_set_one_text(index + 1, c); // +1 because existing API uses 1-based index
}

void VfdDisplay::clear() {
    if (!powered) return;
    vfd_gui_clear();
}

void VfdDisplay::setIcon(DisplayIcon icon, bool enabled) {
    if (!powered) return;
    
    uint32_t iconValue = convertIconEnum(icon);
    vfd_gui_set_pic(iconValue, enabled);
}

void VfdDisplay::clearIcons() {
    if (!powered) return;
    vfd_gui_set_icon(ICON_NONE);
}

void VfdDisplay::setBrightness(uint8_t level) {
    if (level > 7) level = 7;
    brightness = level;
    vfd_gui_set_blk_level(brightness);
}

void VfdDisplay::setColon(uint8_t colonNumber, bool enabled) {
    if (!powered || colonNumber > 1) return;
    
    colonStates[colonNumber] = enabled;
    if (colonNumber == 0) {
        vfd_gui_set_maohao1(enabled ? 1 : 0);
    } else {
        vfd_gui_set_maohao2(enabled ? 1 : 0);
    }
}

void VfdDisplay::powerOn() {
    if (!powered) {
        powered = true;
        vfd_gui_init();
        vfd_gui_set_blk_level(brightness);
    }
}

void VfdDisplay::powerOff() {
    if (powered) {
        powered = false;
        vfd_gui_stop();
    }
}

uint32_t VfdDisplay::convertIconEnum(DisplayIcon icon) {
    return static_cast<uint32_t>(icon);
}
```

## Step 4: Create the State Manager

### 4.1 StateManager.h

```cpp
#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <memory>
#include <unordered_map>
#include <stack>
#include "State.h"

class Application;

class StateManager {
private:
    Application* app;
    std::unordered_map<StateType, std::unique_ptr<State>> states;
    State* currentState;
    std::stack<StateType> stateHistory;
    
public:
    explicit StateManager(Application* application);
    ~StateManager();
    
    // State registration
    void registerState(StateType type, std::unique_ptr<State> state);
    
    // State transitions
    void changeState(StateType newState);
    void pushState(StateType newState);
    void popState();
    
    // Update current state
    void update();
    
    // Event forwarding
    void handleButtonEvent(ButtonEvent event);
    void handleNetworkStateChange(bool connected);
    void handleTimeSync();
    
    // Getters
    State* getCurrentState() { return currentState; }
    StateType getCurrentStateType() const;
};

#endif // STATE_MANAGER_H
```

### 4.2 StateManager.cpp

```cpp
#include "StateManager.h"
#include <Arduino.h> // for Serial

StateManager::StateManager(Application* application) 
    : app(application), currentState(nullptr) {
}

StateManager::~StateManager() {
    if (currentState) {
        currentState->onExit();
    }
}

void StateManager::registerState(StateType type, std::unique_ptr<State> state) {
    states[type] = std::move(state);
}

void StateManager::changeState(StateType newState) {
    auto it = states.find(newState);
    if (it == states.end()) {
        Serial.println("Error: State not found!");
        return;
    }
    
    // Exit current state
    if (currentState) {
        Serial.print("Exiting state: ");
        Serial.println(currentState->getName());
        currentState->onExit();
    }
    
    // Clear history on direct state change
    while (!stateHistory.empty()) {
        stateHistory.pop();
    }
    
    // Enter new state
    currentState = it->second.get();
    Serial.print("Entering state: ");
    Serial.println(currentState->getName());
    currentState->onEnter();
}

void StateManager::pushState(StateType newState) {
    if (currentState) {
        stateHistory.push(getCurrentStateType());
        currentState->onExit();
    }
    
    auto it = states.find(newState);
    if (it != states.end()) {
        currentState = it->second.get();
        currentState->onEnter();
    }
}

void StateManager::popState() {
    if (!stateHistory.empty()) {
        StateType previousState = stateHistory.top();
        stateHistory.pop();
        changeState(previousState);
    }
}

void StateManager::update() {
    if (currentState) {
        currentState->onUpdate();
    }
}

void StateManager::handleButtonEvent(ButtonEvent event) {
    if (currentState) {
        currentState->onButtonEvent(event);
    }
}

StateType StateManager::getCurrentStateType() const {
    if (currentState) {
        return currentState->getType();
    }
    return StateType::TIME; // Default
}
```

## Step 5: Create Your First State - TimeState

### 5.1 TimeState.h

```cpp
#ifndef TIME_STATE_H
#define TIME_STATE_H

#include "app/states/State.h"
#include <Arduino.h>

class TimeState : public State {
private:
    unsigned long lastUpdateTime;
    bool colonVisible;
    
public:
    explicit TimeState(Application* app);
    
    void onEnter() override;
    void onExit() override;
    void onUpdate() override;
    void onButtonEvent(ButtonEvent event) override;
    
    StateType getType() const override { return StateType::TIME; }
    const char* getName() const override { return "Time"; }
};

#endif // TIME_STATE_H
```

### 5.2 TimeState.cpp

```cpp
#include "TimeState.h"
#include "app/Application.h"
#include "services/TimeService.h"
#include <time.h>

TimeState::TimeState(Application* application) 
    : State(application), lastUpdateTime(0), colonVisible(true) {
}

void TimeState::onEnter() {
    app->getDisplay()->clear();
    app->getDisplay()->setIcon(DisplayIcon::CLOCK, true);
}

void TimeState::onExit() {
    app->getDisplay()->setIcon(DisplayIcon::CLOCK, false);
}

void TimeState::onUpdate() {
    unsigned long currentTime = millis();
    
    // Update every 500ms
    if (currentTime - lastUpdateTime >= 500) {
        lastUpdateTime = currentTime;
        
        TimeService* timeService = app->getTimeService();
        if (timeService && timeService->isTimeSynced()) {
            // Get current time
            time_t now;
            tm timeinfo;
            time(&now);
            localtime_r(&now, &timeinfo);
            
            // Format time string
            char buffer[10];
            strftime(buffer, sizeof(buffer), "%H%M%S", &timeinfo);
            
            // Update display
            app->getDisplay()->setText(buffer);
            
            // Toggle colons
            app->getDisplay()->setColon(0, colonVisible);
            app->getDisplay()->setColon(1, colonVisible);
            colonVisible = !colonVisible;
        } else {
            app->getDisplay()->setText("NO NTP");
        }
    }
}

void TimeState::onButtonEvent(ButtonEvent event) {
    if (event == ButtonEvent::SHORT_PRESS) {
        // Short press: Show date or trigger menu item
        // For now, just print to serial
        Serial.println("Short press in Time state");
    } else if (event == ButtonEvent::LONG_PRESS) {
        // Long press: Enter menu
        app->getStateManager()->changeState(StateType::MENU);
    }
}
```

## Step 6: Create the New Main File

### main_new.cpp

```cpp
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
```

## Step 7: Basic Application Implementation

### Application.cpp

```cpp
#include "Application.h"
#include "hal/VfdDisplay.h"
#include "services/TimeService.h"
#include "states/TimeState.h"
#include <Arduino.h>

Application::Application() 
    : lastUpdateTime(0) {
}

Application::~Application() {
    // Cleanup handled by smart pointers
}

bool Application::initialize() {
    Serial.println("Initializing application components...");
    
    // Initialize display
    display = std::make_unique<VfdDisplay>();
    display->powerOn();
    display->setBrightness(2);
    display->setText("BOOT");
    
    // Initialize services
    timeService = std::make_unique<TimeService>();
    
    // Initialize state manager
    stateManager = std::make_unique<StateManager>(this);
    
    // Register states
    stateManager->registerState(StateType::TIME, 
        std::make_unique<TimeState>(this));
    
    // Start with time display
    stateManager->changeState(StateType::TIME);
    
    return true;
}

void Application::update() {
    unsigned long currentTime = millis();
    
    // Update at fixed interval
    if (currentTime - lastUpdateTime >= UPDATE_INTERVAL) {
        lastUpdateTime = currentTime;
        
        // Update current state
        stateManager->update();
        
        // Update services
        if (timeService) {
            timeService->update();
        }
    }
}

void Application::onButtonPress(ButtonEvent event) {
    stateManager->handleButtonEvent(event);
}
```

## Next Steps

1. **Test the basic structure**: 
   - Compile and upload main_new.cpp
   - Verify the display shows "BOOT" then "NO NTP"

2. **Integrate existing functionality**:
   - Add WiFi initialization to NetworkService
   - Move NTP sync to TimeService
   - Create MenuState using existing MenuHandler

3. **Gradual migration**:
   - Keep old main.cpp working
   - Move one feature at a time
   - Test thoroughly after each migration

4. **Add remaining states**:
   - MenuState (from existing menu code)
   - TextScrollState (from animator)
   - ConfigState (from web server)

This approach lets you build the new architecture incrementally while keeping your clock functional throughout the process!