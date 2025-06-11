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