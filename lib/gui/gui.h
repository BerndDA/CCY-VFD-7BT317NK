/***********************************************************************************************
 * Copyright Statement:
 * The copyright of this source code belongs to [saisaiwa].
 *
 * Without the explicit authorization of the copyright owner, no part of this code may be used for commercial purposes, including but not limited to sale, rental, licensing, or publication.
 * It is only for personal learning, research, and non-profit use. If you have other usage needs, please contact
 *[yustart@foxmail.com] for authorization.
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
 * @Date: 2023-07-12 14:11:16
 * @LastEditTime: 2023-08-21 15:18:59
 */
#ifndef __VFD_GUI_
#define __VFD_GUI_

#include "pt6315.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t

#define PIC_REC   0x000800
#define PIC_CLOCK 0x000400
#define PIC_3D    0x000200
#define PIC_WIFI  0x000100

#define ICON_G1_ALL 0xFFFF1F
#define ICON_G1_STYLE_1 0x010000
#define ICON_G1_STYLE_2 0x040000
#define ICON_G1_STYLE_3 0x080000
#define ICON_G1_STYLE_4 0x100000
#define ICON_G1_STYLE_5 0x200000
#define ICON_G1_STYLE_6 0x400000
#define ICON_G1_STYLE_7 0x800000
#define ICON_G1_STYLE_8 0x020000
#define ICON_G1_STYLE_9 0x000100

#define ICON_NONE 0 // Clear all ICON displays

// VFD digit length
#define VFD_DIG_LEN 6

// Filament PWM pin
#define PWM_PIN 13

/**
 * Initialize
 */
void vfd_gui_init();

/**
 * Stop and turn off the display, the filament will stop driving
 */
void vfd_gui_stop();

/**
 * Clear the VFD screen display, loop refresh. If using vfd_gui_set_text method, this is not needed.
 */
void vfd_gui_clear();

/**
 * Display a char character at the specified position, index from 1~6
 */
void vfd_gui_set_one_text(size_t index, char oneChar);

/**
 * Display a string of text starting from position 0.
 * (Automatically clear and overwrite display, convenient to avoid calling clear each time to prevent screen flicker)
 */
u8 vfd_gui_set_text(const char *string);

/**
 * Light up the ICON icon, pass macro definition as parameter
 * @param is_save_state Whether to save this ICON icon to a variable
 */
void vfd_gui_set_icon(u32 buf, u8 is_save_state = 1);

/**
 * Display a picture, the picture is a 3-byte hexadecimal value
 * @param buf use PIC_XXX macro definition
 * @param enabled Whether to display
 */
void vfd_gui_set_pic(u32 buf, bool enabled);

/**
 * Get the saved icon
 */
u32 vfd_gui_get_save_icon(void);

/**
 * Backlight switch
 */
void vfd_gui_set_bck(u8 onOff);

/**
 * Set brightness level 1~7
 */
void vfd_gui_set_blk_level(size_t level);

/**
 * First colon, parameter bool type
 */
void vfd_gui_set_maohao1(u8 open);

/**
 * Second colon, parameter bool type
 */
void vfd_gui_set_maohao2(u8 open);

/**
 * Get font value, internal use
 */
u32 gui_get_font(char c);

#endif