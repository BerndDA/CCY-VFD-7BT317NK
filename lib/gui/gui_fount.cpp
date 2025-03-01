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
 */
const u32 fonts[] = {
    0x848800, // ASCII: ! index: 0
    0x008001, // ASCII: " index: 1
    0x405d01, // ASCII: # index: 2
    0xbe9c1e, // ASCII: $ index: 3
    0x124914, // ASCII: % index: 4
    0x7e0a1b, // ASCII: & index: 5
    0x000001, // ASCII: ' index: 6
    0x1c0212, // ASCII: ( index: 7
    0x2e2004, // ASCII: ) index: 8
    0xc0dd01, // ASCII: * index: 9
    0x809c00, // ASCII: + index: 10
    0x200000, // ASCII: , index: 11
    0x001c00, // ASCII: - index: 12
    0x020000, // ASCII: . index: 13
    0x104904, // ASCII: / index: 14
    0x3e631e, // ASCII: 0 index: 15
    0x222004, // ASCII: 1 index: 16
    0x1e3e1c, // ASCII: 2 index: 17
    0x3e5c1c, // ASCII: 3 index: 18
    0x223c16, // ASCII: 4 index: 19
    0x3e1c1e, // ASCII: 5 index: 20
    0x3e1e1e, // ASCII: 6 index: 21
    0x22201e, // ASCII: 7 index: 22
    0x3e3e1e, // ASCII: 8 index: 23
    0x3e3c1e, // ASCII: 9 index: 24
    0x808000, // ASCII: : index: 25
    0x908000, // ASCII: ; index: 26
    0x405000, // ASCII: < index: 27
    0x1e001c, // ASCII: = index: 28
    0x000901, // ASCII: > index: 29
    0x84481c, // ASCII: ? index: 30
    0x1c3f1e, // ASCII: @ index: 31
    0x323e1e, // ASCII: A index: 32
    0xbeac1c, // ASCII: B index: 33
    0x1e021e, // ASCII: C index: 34
    0xbea81c, // ASCII: D index: 35
    0x1e1e1e, // ASCII: E index: 36
    0x101e1e, // ASCII: F index: 37
    0x3e061e, // ASCII: G index: 38
    0x323e16, // ASCII: H index: 39
    0x9e881c, // ASCII: I index: 40
    0x908a08, // ASCII: J index: 41
    0x525a16, // ASCII: K index: 42
    0x1e0212, // ASCII: L index: 43
    0x326a17, // ASCII: M index: 44
    0x722a17, // ASCII: N index: 45
    0x3e221e, // ASCII: O index: 46
    0x103e1e, // ASCII: P index: 47
    0x7e221e, // ASCII: Q index: 48
    0x523e1e, // ASCII: R index: 49
    0x3e1c1e, // ASCII: S index: 50
    0x84881c, // ASCII: T index: 51
    0x3e2216, // ASCII: U index: 52
    0x804815, // ASCII: V index: 53
    0x722b16, // ASCII: W index: 54
    0x524915, // ASCII: X index: 55
    0x844815, // ASCII: Y index: 56
    0x1e491c, // ASCII: Z index: 57
    0x1e021e, // ASCII: [ index: 58
    0x420811, // ASCII: \ index: 59
    0x3e201c, // ASCII: ] index: 60
    0x408100, // ASCII: ^ index: 61
    0x1e0000, // ASCII: _ index: 62
    0x000001, // ASCII: ` index: 63
};

u32 gui_get_font(char c) {
    if (c == ' ') {
        return 0x00;
    }
    if (c >= 33 && c <= 96) {
        // ! ~ `
        return fonts[map(c, 33, 96, 0, 63)];
    } else if (c >= 97 && c <= 122) {
        //a~z
        return gui_get_font(c - 32);
    } else {
        return 0;
    }
}
