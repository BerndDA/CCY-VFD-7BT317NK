// app/states/ConfigState.h
#ifndef CONFIG_STATE_H
#define CONFIG_STATE_H

#include "app/states/State.h"
#include <string>

class ConfigState : public State {
private:
    std::string password;
    unsigned long lastUpdateTime;
    int scrollPosition;
    
public:
    explicit ConfigState(Application* app);
    
    void onEnter() override;
    void onExit() override;
    void onUpdate() override;
    void onButtonEvent(ButtonEvent event) override;
    
    StateType getType() const override { return StateType::CONFIG; }
    const char* getName() const override { return "Config"; }
    
    void setPassword(const std::string& pwd) { password = pwd; }
};

#endif // CONFIG_STATE_H