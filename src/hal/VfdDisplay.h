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