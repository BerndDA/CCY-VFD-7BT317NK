/***********************************************************************************************
 * Copyright Statement:
 * The copyright of this source code belongs to [saisaiwa].
 *
 * Without the explicit authorization of the copyright owner, no part of this code may be used for commercial purposes, including but not limited to sale, rental, licensing, or publication.
 * It is only for personal learning, research, and non-profit use. If you have other usage needs, please contact [yustart@foxmail.com] for authorization.
 *
 * You are free to modify, use, and distribute this code under the following conditions:
 * - You must retain all content of this copyright statement.
 * - You must not use it for any illegal, abusive, or malicious activities.
 * - Your use should not harm the reputation of any individual, organization, or author.
 *
 * The author does not assume any warranty liability arising from the use of this code. The author is not responsible for any direct or indirect losses caused by the use of this code.
 * Github: https://github.com/ccy-studio/CCY-VFD-7BT317NK
 ***********************************************************************************************/

/*
 * @Description:
 * @Author: chenzedeng
 * @Date: 2023-07-04 14:33:25
 * @LastEditTime: 2023-08-11 22:31:57
 */

// blog-address: saisaiwa.com

#ifndef __PT6315__
#define __PT6315__

#define PT_PLATFORM ESPMCU

#if PT_PLATFORM == ESPMCU
#include <Arduino.h>
#endif

#define CLK_PIN_GROUP 0
#define CLK_PIN 12 //12//13//12//13//14
#define DIN_PIN_GROUP 0
#define DIN_PIN 14 //14//14//13//12//12
#define STB_PIN_GROUP 0
#define STB_PIN 13 //13//12//14//14//13

#if PT_PLATFORM == ESPMCU

#define CLK_1 digitalWrite(CLK_PIN, HIGH)
#define CLK_0 digitalWrite(CLK_PIN, LOW)
#define DIN_1 digitalWrite(DIN_PIN, HIGH)
#define DIN_0 digitalWrite(DIN_PIN, LOW)
#define STB_1 digitalWrite(STB_PIN, HIGH)
#define STB_0 digitalWrite(STB_PIN, LOW)

#define delay_us(us) delayMicroseconds(us)

#endif

#if PT_PLATFORM == STM32
// TODO: Configure STM32 macros
#endif

/**
 * Initialize GPIO
 */
void ptInitGPIO(void);

/**
 * Display control command COMMANDS 4
 * @param onOff 0 to turn off display, 1 to turn on display
 * @param brightnessVal Brightness duty cycle adjustment 0~7
 * 000: Pulse width = 1/16 0
 * 001: Pulse width = 2/16 1
 * 010: Pulse width = 4/16 0x2
 * 011: Pulse width = 10/16 3
 * 100: Pulse width = 11/16 4
 * 101: Pulse width = 12/16 0x5
 * 110: Pulse width = 13/16 6
 * 111: Pulse width = 14/16 0x7
 */
void ptSetDisplayLight(uint8_t onOff, uint8_t brightnessVal);
void setModeWirteDisplayMode(uint8_t addressMode = 0);
void setDisplayMode(uint8_t digit);
void sendDigAndData(uint8_t dig, const uint8_t* data, size_t len);
#endif