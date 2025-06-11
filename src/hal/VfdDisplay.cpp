#include "VfdDisplay.h"
#include "gui.h" // Your existing GUI functions

VfdDisplay::VfdDisplay() 
    : brightness(2), activeIcons(0), powered(true) {
    colonStates[0] = false;
    colonStates[1] = false;
}

VfdDisplay::~VfdDisplay() {
    powerOff();
}

void VfdDisplay::setText(const std::string& text) {
    if (!powered) return;
    
    // Use existing vfd_gui_set_text
    vfd_gui_set_text(text.c_str());
}

void VfdDisplay::setCharAt(size_t index, char c) {
    if (!powered || index >= 6) return;
    
    // Use existing vfd_gui_set_one_text
    vfd_gui_set_one_text(index + 1, c); // +1 because existing API uses 1-based index
}

void VfdDisplay::clear() {
    if (!powered) return;
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
    vfd_gui_set_blk_level(brightness);
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
    if (!powered) {
        powered = true;
        vfd_gui_init();
        vfd_gui_set_blk_level(brightness);
    }
}

void VfdDisplay::powerOff() {
    if (powered) {
        powered = false;
        vfd_gui_stop();
    }
}

uint32_t VfdDisplay::convertIconEnum(DisplayIcon icon) {
    return static_cast<uint32_t>(icon);
}