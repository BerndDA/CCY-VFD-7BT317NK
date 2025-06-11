// hal/VfdDisplay.cpp - Working version
#include "VfdDisplay.h"
#include "gui.h" // Your existing GUI functions

VfdDisplay::VfdDisplay() 
    : brightness(2), activeIcons(0), powered(false) {
    colonStates[0] = false;
    colonStates[1] = false;
    Serial.println("VfdDisplay: Constructor called");
}

VfdDisplay::~VfdDisplay() {
    powerOff();
}

void VfdDisplay::setText(const std::string& text) {
    if (!powered) {
        Serial.println("VfdDisplay::setText - WARNING: Display not powered!");
        return;
    }

    // Use existing vfd_gui_set_text directly
    vfd_gui_set_text(text.c_str());
}

void VfdDisplay::setCharAt(size_t index, char c) {
    if (!powered || index >= 6) return;
    
    // Use existing vfd_gui_set_one_text
    vfd_gui_set_one_text(index + 1, c); // +1 because existing API uses 1-based index
}

void VfdDisplay::clear() {
    if (!powered) {
        Serial.println("VfdDisplay::clear - WARNING: Display not powered!");
        return;
    }
    Serial.println("VfdDisplay::clear");
    vfd_gui_clear();
}

void VfdDisplay::setIcon(DisplayIcon icon, bool enabled) {
    if (!powered) return;
    
    uint32_t iconValue = convertIconEnum(icon);
    vfd_gui_set_pic(iconValue, enabled);
}

void VfdDisplay::clearIcons() {
    if (!powered) return;
    vfd_gui_set_icon(ICON_NONE);
}

void VfdDisplay::setBrightness(uint8_t level) {
    if (level > 7) level = 7;
    brightness = level;
    Serial.print("VfdDisplay::setBrightness - Level: ");
    Serial.println(brightness);
    
    if (powered) {
        vfd_gui_set_blk_level(brightness);
    }
}

void VfdDisplay::setColon(uint8_t colonNumber, bool enabled) {
    if (!powered || colonNumber > 1) return;
    
    colonStates[colonNumber] = enabled;
    if (colonNumber == 0) {
        vfd_gui_set_maohao1(enabled ? 1 : 0);
    } else {
        vfd_gui_set_maohao2(enabled ? 1 : 0);
    }
}

void VfdDisplay::powerOn() {
    Serial.println("VfdDisplay::powerOn - Starting...");
    
    if (powered) {
        Serial.println("VfdDisplay::powerOn - Already powered");
        return;
    }
    
    // Initialize the VFD hardware
    vfd_gui_init();
    
    // Small delay for hardware stabilization
    delay(100);
    // Set brightness
    vfd_gui_set_blk_level(brightness);
    
    // Ensure backlight is on
    vfd_gui_set_bck(1);
    
    powered = true;
}

void VfdDisplay::powerOff() {
    Serial.println("VfdDisplay::powerOff");
    if (powered) {
        vfd_gui_stop();
        powered = false;
    }
}

uint32_t VfdDisplay::convertIconEnum(DisplayIcon icon) {
    return static_cast<uint32_t>(icon);
}