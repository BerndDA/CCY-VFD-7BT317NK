// app/states/TimeState.h
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
    void onNetworkStateChange(bool connected) override;
    void onTimeSync() override;
    
    StateType getType() const override { return StateType::TIME; }
    const char* getName() const override { return "Time"; }
};

#endif // TIME_STATE_H