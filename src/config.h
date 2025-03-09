#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin Definitions
#define KEY1 D3 // Assuming D5 is the pin for KEY1, adjust if needed

// Time Constants
#define MY_NTP_SERVER "at.pool.ntp.org"
#define MY_TZ "CET-1CEST,M3.5.0/02,M10.5.0/03"

// Input Constants
#define LONG_PRESS_TIME 500 // 500ms threshold for long press
#define SCROLL_INTERVAL 800 // 800ms interval for menu scrolling

// Display Constants
#define VFD_TIME_FRAME 500 // Time refresh interval in ms
#define STYLE_TIME 0
#define STYLE_TEXT 1
#define STYLE_MENU 2
#define STYLE_NOTIME 3

// Type definitions for better readability
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Global application state
extern u8 power;       // Power state
extern u8 countdown;   // Countdown state
extern u8 light_level; // Display brightness level
//extern volatile u8 style_page;  // Current page style

#endif // CONFIG_H