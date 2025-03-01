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
 * @Date: 2023-07-04 14:33:32
 * @LastEditTime: 2023-08-11 23:22:20
 */
#include "pt6315.h"

void ptInitGPIO(void) {
#if PT_PLATFORM == ESPMCU
    pinMode(CLK_PIN, OUTPUT);
    pinMode(DIN_PIN, OUTPUT);
    pinMode(STB_PIN, OUTPUT);
#endif
}

void writeData(uint8_t data, int delayState = 1) {
    // CLK rising edge will read serial data, DIN starts from the least significant bit (LSB)
    // CLK PWCLK (Clock Pulse Width) ≥ 400 ns
    // DIN hold time ≥ 100 ns, setup time ≥ 100 ns, total > 200 ns
    CLK_0;
    for (int i = 0; i < 8; i++) {
        delay_us(10);
        if (data & 0x01) {
            DIN_1;
        } else {
            DIN_0;
        }
        delay_us(10);
        CLK_1;
        delay_us(10);
        CLK_0;
        data >>= 1;
    }
    if (delayState) {
        delay_us(10);
    }
}

/**
 * DATA SETTING COMMANDS 2
 * @param addressMode Address mode 0 for auto-increment, 1 for fixed address mode
 */
void setModeWirteDisplayMode(uint8_t addressMode) {
    uint8_t command = 0x40;
    if (addressMode) {
        command |= 0x4;
    }
    STB_1;
    delay_us(10);
    STB_0;
    delay_us(10);
    writeData(command);
    delay_us(10);
    STB_1;
}

/**
 * COMMANDS 1 Display mode setting command
 * 0000: 4 digits, 24 segments
 * 0001: 5 digits, 23 segments
 * 0010: 6 digits, 22 segments
 * 0011: 7 digits, 21 segments
 * 0100: 8 digits, 20 segments
 * 0101: 9 digits, 19 segments
 * 0110: 10 digits, 18 segments
 * 0111: 11 digits, 17 segments
 * 1XXX: 12 digits, 16 segments
 */
void setDisplayMode(uint8_t digit) {
    STB_1;
    delay_us(10);
    STB_0;
    delay_us(10);
    writeData(digit);
    delay_us(10);
    STB_1;
}

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
void ptSetDisplayLight(uint8_t onOff, uint8_t brightnessVal) {
    uint8_t command = 0x80 | brightnessVal;
    if (onOff) {
        command |= 0x8;
    }
    STB_1;
    delay_us(10);
    STB_0;
    delay_us(10);
    // 0x8f
    writeData(command);
    delay_us(10);
    STB_1;
}

void sendDigAndData(uint8_t dig, const uint8_t* data, size_t len) {
    STB_1;
    delay_us(10);
    STB_0;
    delay_us(10);
    writeData(0xc0 | dig);
    delay_us(10);
    // Write data
    for (size_t i = 0; i < len; i++) {
        writeData(data[i], 0);
    }
    delay_us(10);
    STB_1;
}
