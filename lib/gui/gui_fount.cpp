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
 * @Date: 2023-07-12 15:36:22
 * @LastEditTime: 2023-08-12 14:53:59
 */

/*
    Reference for Chinese character images:
    https://blog.csdn.net/fcz1116/article/details/124371360
    https://zhuanlan.zhihu.com/p/94972448
*/

#include "gui.h"

/**
 * The following characters are generated using the bit mapping defined in bit_mapping.txt
 * and the character definitions from font_modified.txt.
 *
 * Each character is defined using the segment definitions (SEG_P0 through SEG_P20)
 * from gui.h instead of hardcoded hexadecimal values.
 */
const u32 fonts[] = {
    SEG_P5 | SEG_P9 | SEG_P13 | SEG_P18,                                                                                                // ASCII: ! index: 0
    SEG_P4 | SEG_P5,                                                                                                                    // ASCII: " index: 1
    SEG_P4 | SEG_P6 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P12 | SEG_P14,                                                                    // ASCII: # index: 2
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P5 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P13 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19, // ASCII: $ index: 3
    SEG_P0 | SEG_P2 | SEG_P6 | SEG_P9 | SEG_P12 | SEG_P16 | SEG_P19,                                                                    // ASCII: % index: 4
    SEG_P0 | SEG_P1 | SEG_P3 | SEG_P4 | SEG_P9 | SEG_P11 | SEG_P14 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                   // ASCII: & index: 5
    SEG_P4,                                                                                                                             // ASCII: ' index: 6
    SEG_P0 | SEG_P3 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18,                                                                            // ASCII: ( index: 7
    SEG_P2 | SEG_P7 | SEG_P15 | SEG_P17 | SEG_P18 | SEG_P19,                                                                            // ASCII: ) index: 8
    SEG_P4 | SEG_P5 | SEG_P6 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P12 | SEG_P13 | SEG_P14,                                                 // ASCII: * index: 9
    SEG_P5 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P13,                                                                                       // ASCII: + index: 10
    SEG_P15,                                                                                                                            // ASCII: , index: 11
    SEG_P8 | SEG_P9 | SEG_P10,                                                                                                          // ASCII: - index: 12
    SEG_P19,                                                                                                                            // ASCII: . index: 13
    SEG_P2 | SEG_P6 | SEG_P9 | SEG_P12 | SEG_P16,                                                                                       // ASCII: / index: 14
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P6 | SEG_P7 | SEG_P11 | SEG_P12 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,          // ASCII: 0 index: 15
    SEG_P2 | SEG_P7 | SEG_P15 | SEG_P19,                                                                                                // ASCII: 1 index: 16
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: 2 index: 17
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P6 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: 3 index: 18
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P15 | SEG_P19,                                                  // ASCII: 4 index: 19
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: 5 index: 20
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,          // ASCII: 6 index: 21
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P15 | SEG_P19,                                                                     // ASCII: 7 index: 22
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19, // ASCII: 8 index: 23
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,           // ASCII: 9 index: 24
    SEG_P5 | SEG_P13,                                                                                                                   // ASCII: : index: 25
    SEG_P5 | SEG_P13 | SEG_P16,                                                                                                         // ASCII: ; index: 26
    SEG_P6 | SEG_P8 | SEG_P14,                                                                                                          // ASCII: < index: 27
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                                   // ASCII: = index: 28
    SEG_P4 | SEG_P9 | SEG_P12,                                                                                                          // ASCII: > index: 29
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P6 | SEG_P9 | SEG_P13 | SEG_P18,                                                                     // ASCII: ? index: 30
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P12 | SEG_P16 | SEG_P17 | SEG_P18,           // ASCII: @ index: 31
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P19,                     // ASCII: A index: 32
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P5 | SEG_P7 | SEG_P9 | SEG_P10 | SEG_P13 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,          // ASCII: B index: 33
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                // ASCII: C index: 34
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P5 | SEG_P7 | SEG_P9 | SEG_P13 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: D index: 35
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: E index: 36
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P16,                                                  // ASCII: F index: 37
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P10 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                            // ASCII: G index: 38
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P19,                              // ASCII: H index: 39
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P5 | SEG_P9 | SEG_P13 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                       // ASCII: I index: 40
    SEG_P1 | SEG_P5 | SEG_P9 | SEG_P11 | SEG_P13 | SEG_P16 | SEG_P17,                                                                   // ASCII: J index: 41
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P6 | SEG_P8 | SEG_P9 | SEG_P11 | SEG_P14 | SEG_P16 | SEG_P19,                                        // ASCII: K index: 42
    SEG_P0 | SEG_P3 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                                  // ASCII: L index: 43
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P4 | SEG_P6 | SEG_P7 | SEG_P9 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P19,                               // ASCII: M index: 44
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P4 | SEG_P7 | SEG_P9 | SEG_P11 | SEG_P14 | SEG_P15 | SEG_P16 | SEG_P19,                              // ASCII: N index: 45
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                             // ASCII: O index: 46
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P16,                                         // ASCII: P index: 47
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P11 | SEG_P14 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                   // ASCII: Q index: 48
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P11 | SEG_P14 | SEG_P16 | SEG_P19,                     // ASCII: R index: 49
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P8 | SEG_P9 | SEG_P10 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                    // ASCII: S index: 50
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P5 | SEG_P9 | SEG_P13 | SEG_P18,                                                                     // ASCII: T index: 51
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P11 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                      // ASCII: U index: 52
    SEG_P0 | SEG_P2 | SEG_P4 | SEG_P6 | SEG_P9 | SEG_P13,                                                                               // ASCII: V index: 53
    SEG_P0 | SEG_P2 | SEG_P3 | SEG_P7 | SEG_P9 | SEG_P11 | SEG_P12 | SEG_P14 | SEG_P15 | SEG_P16 | SEG_P19,                             // ASCII: W index: 54
    SEG_P0 | SEG_P2 | SEG_P4 | SEG_P6 | SEG_P9 | SEG_P12 | SEG_P14 | SEG_P16 | SEG_P19,                                                 // ASCII: X index: 55
    SEG_P0 | SEG_P2 | SEG_P4 | SEG_P6 | SEG_P9 | SEG_P13 | SEG_P18,                                                                     // ASCII: Y index: 56
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P6 | SEG_P9 | SEG_P12 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                       // ASCII: Z index: 57
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P3 | SEG_P11 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                // ASCII: [ index: 58
    SEG_P0 | SEG_P4 | SEG_P9 | SEG_P14 | SEG_P19,                                                                                       // ASCII: \ index: 59
    SEG_P0 | SEG_P1 | SEG_P2 | SEG_P7 | SEG_P15 | SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                // ASCII: ] index: 60
    SEG_P5 | SEG_P12 | SEG_P14,                                                                                                         // ASCII: ^ index: 61
    SEG_P16 | SEG_P17 | SEG_P18 | SEG_P19,                                                                                              // ASCII: _ index: 62
    SEG_P4,                                                                                                                             // ASCII: ` index: 63
};

u32 gui_get_font(char c)
{
    if (c == ' ')
    {
        return 0x00;
    }
    if (c >= 33 && c <= 96)
    {
        // ! ~ `
        return fonts[map(c, 33, 96, 0, 63)];
    }
    else if (c >= 97 && c <= 122)
    {
        // a~z
        return gui_get_font(c - 32);
    }
    else
    {
        return 0;
    }
}