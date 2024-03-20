#ifndef KEYBOARD_H
#define KEYBOARD_H

/*
第一列10进制键值，第二列16进制键值，第四列是按键

// 0 00 Reserved (no event indicated)9 N/A √ √ √ 4/101/104
// 1 01 Keyboard ErrorRollOver9 N/A √ √ √ 4/101/104
// 2 02 Keyboard POSTFail9 N/A √ √ √ 4/101/104
// 3 03 Keyboard ErrorUndefined9 N/A √ √ √ 4/101/104
4 04 Keyboard a and A4 31 √ √ √ 4/101/104
5 05 Keyboard b and B 50 √ √ √ 4/101/104
6 06 Keyboard c and C4 48 √ √ √ 4/101/104
7 07 Keyboard d and D 33 √ √ √ 4/101/104
8 08 Keyboard e and E 19 √ √ √ 4/101/104
9 09 Keyboard f and F 34 √ √ √ 4/101/104
10 0A Keyboard g and G 35 √ √ √ 4/101/104
11 0B Keyboard h and H 36 √ √ √ 4/101/104
12 0C Keyboard i and I 24 √ √ √ 4/101/104
13 0D Keyboard j and J 37 √ √ √ 4/101/104
14 0E Keyboard k and K 38 √ √ √ 4/101/104
15 0F Keyboard l and L 39 √ √ √ 4/101/104
16 10 Keyboard m and M4 52 √ √ √ 4/101/104
17 11 Keyboard n and N 51 √ √ √ 4/101/104
18 12 Keyboard o and O4 25 √ √ √ 4/101/104
19 13 Keyboard p and P4 26 √ √ √ 4/101/104
20 14 Keyboard q and Q4 17 √ √ √ 4/101/104
21 15 Keyboard r and R 20 √ √ √ 4/101/104
22 16 Keyboard s and S4 32 √ √ √ 4/101/104
23 17 Keyboard t and T 21 √ √ √ 4/101/104
24 18 Keyboard u and U 23 √ √ √ 4/101/104
25 19 Keyboard v and V 49 √ √ √ 4/101/104
26 1A Keyboard w and W4 18 √ √ √ 4/101/104
27 1B Keyboard x and X4 47 √ √ √ 4/101/104
28 1C Keyboard y and Y4 22 √ √ √ 4/101/104
29 1D Keyboard z and Z4 46 √ √ √ 4/101/104
30 1E Keyboard 1 and !4 2 √ √ √ 4/101/104
31 1F Keyboard 2 and @4 3 √ √ √ 4/101/104
32 20 Keyboard 3 and #4 4 √ √ √ 4/101/104
33 21 Keyboard 4 and $4 5 √ √ √ 4/101/104
34 22 Keyboard 5 and %4 6 √ √ √ 4/101/104
35 23 Keyboard 6 and ^4 7 √ √ √ 4/101/104
36 24 Keyboard 7 and &4 8 √ √ √ 4/101/104
37 25 Keyboard 8 and *4 9 √ √ √ 4/101/104
38 26 Keyboard 9 and (4 10 √ √ √ 4/101/104
39 27 Keyboard 0 and )4 11 √ √ √ 4/101/104
40 28 Keyboard Return (ENTER)5 43 √ √ √ 4/101/104
41 29 Keyboard ESCAPE 110 √ √ √ 4/101/104
42 2A Keyboard DELETE (Backspace)13 15 √ √ √ 4/101/104
43 2B Keyboard Tab 16 √ √ √ 4/101/104
44 2C Keyboard Spacebar 61 √ √ √ 4/101/104
45 2D Keyboard - and (underscore)4 12 √ √ √ 4/101/104
46 2E Keyboard = and +4 13 √ √ √ 4/101/104
47 2F Keyboard [ and {4 27 √ √ √ 4/101/104
48 30 Keyboard ] and }4 28 √ √ √ 4/101/104
49 31 Keyboard \ and | 29 √ √ √ 4/101/104
50 32 Keyboard Non-US # and ~2 42 √ √ √ 4/101/104
51 33 Keyboard ; and :4 40 √ √ √ 4/101/104
52 34 Keyboard ‘ and “4 41 √ √ √ 4/101/104
54 36 Keyboard, and <4 53 √ √ √ 4/101/104
55 37 Keyboard . and >4 54 √ √ √ 4/101/104
56 38 Keyboard / and ?4 55 √ √ √ 4/101/104
57 39 Keyboard Caps Lock11 30 √ √ √ 4/101/104
58 3A Keyboard F1 112 √ √ √ 4/101/104
59 3B Keyboard F2 113 √ √ √ 4/101/104
60 3C Keyboard F3 114 √ √ √ 4/101/104
61 3D Keyboard F4 115 √ √ √ 4/101/104
62 3E Keyboard F5 116 √ √ √ 4/101/104
63 3F Keyboard F6 117 √ √ √ 4/101/104
64 40 Keyboard F7 118 √ √ √ 4/101/104
65 41 Keyboard F8 119 √ √ √ 4/101/104
66 42 Keyboard F9 120 √ √ √ 4/101/104
67 43 Keyboard F10 121 √ √ √ 4/101/104
68 44 Keyboard F11 122 √ √ √ 101/104
69 45 Keyboard F12 123 √ √ √ 101/104
70 46 Keyboard PrintScreen1 124 √ √ √ 101/104
71 47 Keyboard Scroll Lock11 125 √ √ √ 4/101/104
73 49 Keyboard Insert1 75 √ √ √ 101/104
74 4A Keyboard Home1 80 √ √ √ 101/104
75 4B Keyboard PageUp1 85 √ √ √ 101/104
76 4C Keyboard Delete Forward1;14 76 √ √ √ 101/104
77 4D Keyboard End1 81 √ √ √ 101/104
78 4E Keyboard PageDown1 86 √ √ √ 101/104
79 4F Keyboard RightArrow1 89 √ √ √ 101/104
80 50 Keyboard LeftArrow1 79 √ √ √ 101/104
81 51 Keyboard DownArrow1 84 √ √ √ 101/104
82 52 Keyboard UpArrow1 83 √ √ √ 101/104
83 53 Keypad Num Lock and Clear11 90 √ √ √ 101/104
84 54 Keypad /1 95 √ √ √ 101/104
85 55 Keypad * 100 √ √ √ 4/101/104
86 56 Keypad - 105 √ √ √ 4/101/104
87 57 Keypad + 106 √ √ √ 4/101/104
88 58 Keypad ENTER5 108 √ √ √ 101/104
89 59 Keypad 1 and End 93 √ √ √ 4/101/104
90 5A Keypad 2 and Down Arrow 98 √ √ √ 4/101/104
91 5B Keypad 3 and PageDn 103 √ √ √ 4/101/104
92 5C Keypad 4 and Left Arrow 92 √ √ √ 4/101/104
93 5D Keypad 5 97 √ √ √ 4/101/104
94 5E Keypad 6 and Right Arrow 102 √ √ √ 4/101/104
95 5F Keypad 7 and Home 91 √ √ √ 4/101/104
96 60 Keypad 8 and Up Arrow 96 √ √ √ 4/101/104
97 61 Keypad 9 and PageUp 101 √ √ √ 4/101/104
98 62 Keypad 0 and Insert 99 √ √ √ 4/101/104
99 63 Keypad . and Delete 104 √ √ √ 4/101/104
100 64 Keyboard Non-US \ and |3;6 45 √ √ √ 4/101/104
101 65 Keyboard Application10 129 √ √ 104 //fn?
224 E0 Keyboard LeftControl 58 √ √ √ 4/101/104
225 E1 Keyboard LeftShift 44 √ √ √ 4/101/104
226 E2 Keyboard LeftAlt 60 √ √ √ 4/101/104
227 E3 Keyboard Left GUI10;23 127 √ √ √ 104
228 E4 Keyboard RightControl 64 √ √ √ 101/104
229 E5 Keyboard RightShift 57 √ √ √ 4/101/104
230 E6 Keyboard RightAlt 62 √ √ √ 101/104
231 E7 Keyboard Right GUI10;24 128 √ √ √ 104
*/

#include "hid_core.h"
#include "usb_user.h"


void scan_keyboard(void);

#endif //   KEYBOARD_H
