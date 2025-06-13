// app/states/TimeState.h
#ifndef TIME_STATE_H
#define TIME_STATE_H

#include "app/states/State.h"
#include <memory>
#include <Arduino.h>

// Forward declaration
class Animator;

class TimeState : public State {
private:
    unsigned long lastUpdateTime;
    bool colonVisible;
    std::unique_ptr<Animator> animator;
    bool isAnimating;
    int lastSecond;
    bool longPressHandled; 
    // For date display animation
    struct tm savedTimeInfo;
    
public:
    explicit TimeState(Application* app);
    ~TimeState();
    
    void onEnter() override;
    void onExit() override;
    void onUpdate() override;
    void onButtonEvent(ButtonEvent event) override;
    void onNetworkStateChange(bool connected) override;
    void onTimeSync() override;
    
    StateType getType() const override { return StateType::TIME; }
    const char* getName() const override { return "Time"; }
    
private:
    void showDateAnimation();
    void updateTimeDisplay();
};

#endif // TIME_STATE_H