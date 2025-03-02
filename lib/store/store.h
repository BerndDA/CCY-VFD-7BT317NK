/***********************************************************************************************
 * Copyright Statement:
 * The copyright of this source code belongs to [saisaiwa].
 *
 * Without the explicit authorization of the copyright owner, no part of this code may be used for commercial purposes, including but not limited to sale, rental, licensing, or publication.
 * It is only for personal learning, research, and non-profit use. If you have other usage needs, please contact [yustart@foxmail.com] for authorization.
 *
 * You are free to modify, use, and distribute this code under the following conditions:
 * - You must retain all content of this copyright statement.
 * - You may not use it for any illegal, abusive, or malicious activities.
 * - Your use should not damage the reputation of any individual, organization, or author.
 *
 * The author does not assume any warranty liability arising from the use of this code. The author is not responsible for any direct or indirect losses caused by the use of this code.
 * Github: https://github.com/ccy-studio/CCY-VFD-7BT317NK
 ***********************************************************************************************/

/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-08-18 15:43:27
 * @LastEditTime: 2023-08-21 21:12:37
 */
#ifndef __STORE_H
#define __STORE_H

#include <Arduino.h>
#include <constant.h>

typedef struct {
    u8 anno_open = 1;         // G1 animation switch

    char custom_long_text[50] = {"Hello VFD"};  // Scrolling text
    u8 custom_long_text_frame = 255;            // Scrolling text frame rate

    u8 auto_power;                  // Scheduled power on/off switch
    char auto_power_open_time[9];   // Power on time setting
    char auto_power_close_time[9];  // Power off time setting
    u8 auto_power_enable_days[7];   // Conditions for power on/off to take effect

    u8 alarm_clock;                 // Alarm clock switch
    char alarm_clock_time[9];       // Alarm clock time setting
    u8 alarm_clock_enable_days[7];  // Conditions for alarm clock to take effect

    u8 countdown;            // Countdown
    char countdown_time[9];  // Countdown time
} store_setting_obj;

extern store_setting_obj setting_obj;

void store_init();

void store_close();

void store_save_setting(store_setting_obj* obj);

void store_get_setting(store_setting_obj* obj);

void store_del_setting(void);

#ifdef DEBUG
void store_print_debug(store_setting_obj setting_obj);
#endif

#endif