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