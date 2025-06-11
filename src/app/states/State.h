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