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
 * @Author: chenzedeng
 * @Date: 2023-07-12 14:14:04
 * @LastEditTime: 2023-08-21 23:53:01
 */
#include "gui.h"

u8 lightOff = 1;   // Backlight switch
u8 lightLevel = 2; // Brightness level

u8 mh1, mh2; // Colon 1, 2

// Record the current value of the individual ICON setting
u32 current_icon_flag = 0;
u32 save_icon = 0;
u32 current_pic_flag = 0;

void vfd_gui_init()
{
    // Initialize GPIO
    ptInitGPIO();
    pinMode(PWM_PIN, OUTPUT);
    // Set the frequency of PWM in Hz

    // ------------------------------------
    // Frequency used for V2V3 version
    analogWriteFreq(20000);
    analogWrite(PWM_PIN, 25);
    // ------------------------------------
    // Frequency for V1
    //  analogWriteFreq(20000);
    //  analogWrite(PWM_PIN, 15);
    // ------------------------------------
    // VFD Setting
    setDisplayMode(3); // command1
    vfd_gui_clear();
}

void vfd_gui_stop()
{
    vfd_gui_clear();
    current_icon_flag = 0;
    mh1 = 0;
    mh2 = 0;
    digitalWrite(PWM_PIN, LOW);
}

void vfd_gui_clear()
{
    setModeWirteDisplayMode(0); // command2
    u8 clearBuf[24];
    memset(clearBuf, 0x00, sizeof(clearBuf));
    setModeWirteDisplayMode(0);              // command2
    sendDigAndData(0, clearBuf, 24);         // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
}

void vfd_gui_set_one_text(size_t index, char oneChar)
{
    setModeWirteDisplayMode(0); // command2
    u32 buf = gui_get_font(oneChar);
    uint8_t arr[3];
    arr[0] = (buf >> 16) & 0xFF;
    arr[1] = (buf >> 8) & 0xFF;
    arr[2] = buf & 0xFF;
    sendDigAndData(index * 3, arr, 3);       // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
}

void vfd_gui_set_one_pattern(size_t index, u32 pattern)
{
    setModeWirteDisplayMode(0); // command2
    uint8_t arr[3];
    arr[0] = (pattern >> 16) & 0xFF;
    arr[1] = (pattern >> 8) & 0xFF;
    arr[2] = pattern & 0xFF;
    sendDigAndData(index * 3, arr, 3);       // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
}

void vfd_gui_set_icon(u32 buf, u8 is_save_state)
{
    if (current_icon_flag == buf)
    {
        // Filter duplicate submissions
        return;
    }
    uint8_t arr[3];
    arr[0] = (buf >> 16) & 0xFF;
    arr[1] = (buf >> 8) & 0xFF;
    arr[2] = buf & 0xFF;
    setModeWirteDisplayMode(0);              // command2
    sendDigAndData(6 * 3, arr, 3);           // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
    if (is_save_state)
    {
        save_icon = buf;
    }
    current_icon_flag = buf;
}

void vfd_gui_draw_pic(){
    uint8_t arr[3];
    arr[0] = (current_pic_flag >> 16) & 0xFF;
    arr[1] = (current_pic_flag >> 8) & 0xFF;
    arr[2] = current_pic_flag & 0xFF;
    setModeWirteDisplayMode(0);              // command2
    sendDigAndData(6 * 3, arr, 3);           // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
}

void vfd_gui_set_pic(u32 buf, bool enabled)
{
    // add or remove from pic list
    u32 new_pic_flag = enabled ? (current_pic_flag | buf) : (current_pic_flag & ~buf);
    if (new_pic_flag == current_pic_flag) {
        // No change in pic flag, return early
        return;
    }
    current_pic_flag = new_pic_flag;
    vfd_gui_draw_pic();
}

u32 vfd_gui_get_save_icon(void)
{
    return save_icon;
}

u8 vfd_gui_set_text(const char *string)
{
    size_t str_len = strlen(string);
    static u8 data[18];
    memset(data, 0, sizeof(data));
    size_t index = 0;
    for (size_t i = 0; i < str_len && i < 6; i++)
    {
        if (string[i] && string[i] != '\0')
        {
            u32 buf = gui_get_font(string[i]);
            data[index++] = (buf >> 16) & 0xFF;
            data[index++] = (buf >> 8) & 0xFF;
            data[index++] = buf & 0xFF;
        }
    }
    // Handling of colons
    if (str_len >= 2)
    {
        mh1 = data[3];
    }
    else
    {
        mh1 = 0;
    }
    if (str_len >= 4)
    {
        mh2 = data[9];
    }
    else
    {
        mh2 = 0;
    }
    setModeWirteDisplayMode(0);              // command2
    sendDigAndData(0, data, 18);             // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
    return 1;
}

void vfd_gui_set_bck(u8 onOff)
{
    lightOff = onOff;
}

/**
 * Set brightness level 1~7
 */
void vfd_gui_set_blk_level(size_t level)
{
    lightLevel = level;
}

static void vfd_set_maohao(u8 address, u8 buf)
{
    setModeWirteDisplayMode(1);              // command2
    sendDigAndData(address, &buf, 1);        // command3
    ptSetDisplayLight(lightOff, lightLevel); // command4
}

void vfd_gui_set_maohao1(u8 open)
{
    u8 command = open ? mh1 | 0x01 : mh1;
    vfd_set_maohao(0x08 - 5, command);
}
void vfd_gui_set_maohao2(u8 open)
{
    u8 command = open ? mh2 | 0x01 : mh2;
    vfd_set_maohao(0x0E - 5, command);
}