// app/states/MenuState.h - Updated with action execution methods
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
    
    // Methods for menu action execution
    bool hasPendingAction() const;
    void executeSelectedAction();
    
    // Get menu handler for external access (if needed)
    MenuHandler* getMenuHandler() { return menuHandler.get(); }
    
private:
    void displayCurrentMenuItem();
    void scrollToNext();
    void flashMenuItem();
    void startFadeDemo();
};

#endif // MENU_STATE_H