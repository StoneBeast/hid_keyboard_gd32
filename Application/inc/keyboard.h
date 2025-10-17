#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "hid_core.h"
#include "usb_user.h"

#define BUFFER_SIZE 8       //  key buffer max size
#define MX_ROW_COUNT 8      //  real row count in keyboard
#define MX_COL_COUNT 18     //  real col count in keyboard
// #define ROW_OFFSET 2        //  keyboard row n == GPIOA_PIN(n-ROW_OFFSET)
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

#define GPIO_PIN(x) BIT(x)
#define ROW_TO_PIN(row) GPIO_PIN(row+1)
#define COL_TO_PIN(col) (col > 16) ? (GPIO_PIN(col - 17)) : (GPIO_PIN(col - 1))
#define ROW_COL_TO_INDEX(rc)    (rc-1)
#define INEDX_TO_ROW_COL(idx)   (idx+1)

#define GAP_DELAY()                          \
    {                                        \
        volatile uint16_t gap_i = 0;         \
        for (gap_i = 0; gap_i < 30; gap_i++) \
            ;                                \
    }

void scan_keyboard(void);

#endif //   KEYBOARD_H
