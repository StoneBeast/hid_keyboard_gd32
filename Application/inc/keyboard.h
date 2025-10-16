#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "hid_core.h"
#include "usb_user.h"

#define BUFFER_SIZE 8       //  key buffer max size
#define GPIO_PIN(x) BIT(x)

#define MX_ROW_COUNT 8      //  real row count in keyboard
#define MX_COL_COUNT 18     //  real col count in keyboard
#define ROW_OFFSET 2        //  keyboard row n == GPIOA_PIN(n-ROW_OFFSET)
#define FN_KEY_COUNT 12

#define KEY_F1  0x3a
#define KEY_F2  0x3b
#define KEY_F10 0x43
#define KEY_F11 0x44
#define KEY_F12 0x45
#define KEY_PRT_SC 0x46
#define KEY_INS 0x49
#define KEY_DEL 0x4c
#define KEY_UA 0x52
#define KEY_DA 0x51
#define KEY_LA 0x50
#define KEY_RA 0x4f

void scan_keyboard(void);

#endif //   KEYBOARD_H
