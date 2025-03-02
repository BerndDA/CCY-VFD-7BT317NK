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
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2023-08-21 14:57:56
 * @LastEditTime: 2023-08-22 16:29:47
 */
#ifndef __CONSTANT_H
#define __CONSTANT_H

// Uncomment to enable debug mode logging
// #define DEBUG

// Button PINs
#define KEY1 0
#define KEY2 4
#define KEY3 2

/**
 * G1 animation frame rate
 */
#define G1_ANNO_FRAME 190

/**
 * RGB frame rate
 */
#define RGB_ANNO_FRAME 10

/**
 * VFD time refresh frame rate
 */
#define VFD_TIME_FRAME 500

/**
 * Display styles
 */
#define STYLE_DEFAULT 0   // Default time
#define STYLE_CUSTOM_1 1  // Long text scrolling
#define STYLE_CUSTOM_2 2  // WIFI IP address
#define STYLE_TEXT 3      // Text display

#define NTP3 "0.europe.pool.ntp.org"
// #define NTP2 "cn.ntp.org.cn"
// #define NTP1 "ntp.tuna.tsinghua.edu.cn"
#endif