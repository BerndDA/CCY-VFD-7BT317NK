// app/states/MenuState.h
#ifndef MENU_STATE_H
#define MENU_STATE_H

#include "app/states/State.h"
#include <memory>

// Forward declaration
class MenuHandler;

class MenuState : public State {
private:
    std::unique_ptr<MenuHandler> menuHandler;
    unsigned long lastScrollTime;
    bool isScrolling;
    static constexpr unsigned long SCROLL_INTERVAL = 800; // ms between menu items
    
public:
    explicit MenuState(Application* app);
    ~MenuState();
    
    void onEnter() override;
    void onExit() override;
    void onUpdate() override;
    void onButtonEvent(ButtonEvent event) override;
    
    StateType getType() const override { return StateType::MENU; }
    const char* getName() const override { return "Menu"; }
    
private:
    void displayCurrentMenuItem();
    void scrollToNext();
    void selectCurrentItem();
    void flashMenuItem();
    void startFadeDemo();
};

#endif // MENU_STATE_H