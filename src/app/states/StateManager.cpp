// app/states/StateManager.cpp
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

void StateManager::handleNetworkStateChange(bool connected) {
    if (currentState) {
        currentState->onNetworkStateChange(connected);
    }
}

void StateManager::handleTimeSync() {
    if (currentState) {
        currentState->onTimeSync();
    }
}

StateType StateManager::getCurrentStateType() const {
    if (currentState) {
        return currentState->getType();
    }
    return StateType::TIME; // Default
}

State* StateManager::getState(StateType type) {
    auto it = states.find(type);
    if (it != states.end()) {
        return it->second.get();
    }
    return nullptr;
}