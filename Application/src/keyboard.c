#include "keyboard.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "usb_delay.h"


#define BUFFER_SIZE 8
#define GPIO_PIN(x) BIT(x)

/* 实际键盘按键的位置与该按键键值所在位置的索引的映射 */
static uint8_t gs_phy_mx[MX_ROW_COUNT][MX_COL_COUNT] = {
    {1, 112, 113, 6, 7, 13, 119, 80, 85, 75, 76, 0, 120, 12, 0, 58},
    {131, 132, 133, 50, 51, 56, 129, 79, 105, 89, 84, 0, 123, 55, 62, 0},
    {46, 47, 48, 49, 52, 53, 54, 0, 100, 95, 90, 0, 43, 42, 0, 64},
    {110, 45, 115, 35, 36, 117, 0, 83, 104, 99, 61, 0, 122, 41, 60, 0},
    {31, 32, 33, 34, 37, 38, 39, 108, 103, 98, 93, 57, 29, 40, 59, 0},
    {16, 30, 114, 21, 22, 28, 118, 107, 102, 97, 92, 44, 15, 27, 0, 0},
    {17, 18, 19, 20, 23, 24, 25, 106, 101, 96, 91, 0, 14, 26, 125, 126}};

/* 按键键值数组，通过gs_phy_mx中得到的索引在该数组中得到按下按键的键值 */
static uint8_t gs_phy_to_keycode[144] = {
    0, 0x35, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x2d, 0x2e, 0, 0x2a, 0x2b, 0x14, 0x1a,
    0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, 0x12, 0x13, 0x2f,
    0x30, 0x31, 0x39, 0x04, 0x16, 0x07, 0x09, 0x0a, 0x0b,
    0x0d, 0x0e, 0x0f, 0x33, 0x34, 0, 0x28, 0xe1, 0, 0x1d,
    0x1b, 0x06, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37, 0x38, 0,
    0xe1, 0xe4, 0xff, 0xe2, 0x2c, 0xe6, 0, 0xe4, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0x49, 0x4c, 0, 0, 0x50, 0, 0, 0,
    0x52, 0x51, 0, 0, 0, 0, 0x4f, 0x53, 0x24, 0x21, 0x1e, 0,
    0x54, 0x25, 0x22, 0x1f, 0x27, 0x55, 0x26, 0x23, 0x20,
    0x36, 0x56, 0x57, 0, 0x2a, 0, 0x2a, 0, 0x3a, 0x3b,
    0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x68, 0x69,
    0x46, 0, 0, 0xe3, 0, 0x65, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0};

static volatile bool gs_ghosting_flag = FALSE; /* 全局变量，鬼键标志位 */

/* 按键数据缓冲区结构体 */
typedef struct
{
    uint8_t buffer[BUFFER_SIZE];    /* 按下按键的键值 */
    uint8_t key_count;              /* 按下按键的数量 */
    uint8_t normal_key_count;       /* 普通按键的数量，即是不包括ctrl、shift等按键的数量 */
} buffer_t;

extern usb_core_handle_struct usbhs_core_dev;   /* usb设备 */

static uint8_t gs_mx_input_key_buffer[MX_ROW_COUNT][MX_COL_COUNT] = {0};    /* 模拟实际键盘矩阵，方便冲突检测 */
static uint8_t gs_mx_input_key_buffer_count = 0;                            /* 按下按键的数量 */

static uint32_t gs_input_key_buffer[MX_ROW_COUNT] = {0};                    /* 用于消抖 */

//  作为实际发送的key buffer的缓冲
static buffer_t gs_temp_key_buffer = {.buffer = {0}, .key_count = 0, .normal_key_count = 0};

static void handle_input_data(uint8_t row_inx, uint16_t gpio_input_data);
static void handle_original_code(uint8_t row_code, uint8_t col_code);
static bool is_ghosting(uint8_t row_code, uint8_t col_code);
static uint16_t get_col_data(void);

/*!
    \brief      scan the keyboard matrix
    \param[in]  none
    \param[out] none
    \retval     none
*/
void scan_keyboard(void)
{
    // 0000 0000 0000 0000
    // 0000 0011 1111 1000 0X3F8
    gpio_port_write(GPIOA, (gpio_output_port_get(GPIOA) | 0x03f8));

    while (1)
    {
        uint16_t col_data = 0x0000;
        gs_ghosting_flag = FALSE;

        //  这里row_inx对应的时GPIOAx，所以从ROW_OFFSET开始
        for (uint8_t row_inx = ROW_OFFSET; row_inx < (ROW_OFFSET + MX_ROW_COUNT); row_inx++)
        {
            //  逐行扫描，将要扫描的行对应的GPIOA引脚置于低电平
            gpio_bit_reset(GPIOA, GPIO_PIN(row_inx));

            //  获取当前的col输入
            col_data = get_col_data();
            //  处理col data
            handle_input_data(row_inx, col_data);

            //  复位引脚，准备扫描下一行
            gpio_bit_set(GPIOA, GPIO_PIN(row_inx));
        }


        if ((gs_ghosting_flag == FALSE) && (buffer_cmp(gs_temp_key_buffer.buffer) == 0))
        {
            //  将缓冲区的按键数据复制到实际的buffer中并发送
            memcpy(get_key_buffer(), gs_temp_key_buffer.buffer, 8);

            usbd_hid_report_send(&usbhs_core_dev, get_key_buffer(), 8U, EP1_IN);

        }


        memset(gs_mx_input_key_buffer, 0, sizeof(gs_mx_input_key_buffer));
        gs_mx_input_key_buffer_count = 0;
        
        
        memset(gs_temp_key_buffer.buffer, 0, BUFFER_SIZE);
        gs_temp_key_buffer.key_count = 0;
        gs_temp_key_buffer.normal_key_count = 0;
    }
}

/********************************************** 处理扫描的行数据 *************************************************/

void handle_input_data(uint8_t row_inx, uint16_t gpio_input_data)
{
    //  在没有按键按下时，相应位的数据应为 1，反之则为0，这里为了方便计算，将数据全部取反
    gpio_input_data = ~gpio_input_data;

    if (gpio_input_data != gs_input_key_buffer[row_inx - ROW_OFFSET])
    {
        //  消抖，如果5ms后，该列的值仍和第一次检测时相同，则视为有效值
        delay_ms(5);
        if ((gpio_input_data ^ get_col_data()) == 0xffff)
        {
            //  将该值放入数组中
            gs_input_key_buffer[row_inx - ROW_OFFSET] = gpio_input_data;
        }
        else
        {
            //  如果抖动，则将本次获得的值覆盖为上一轮扫描时得到的值
            gpio_input_data = gs_input_key_buffer[row_inx - ROW_OFFSET];
        }
    }

    //  如果没有按键按下，则直接返回，由于没有按键按下也可能是相对上一轮扫描的变化，所以该步骤放在上一个判断之后
    if (gpio_input_data == 0x0000)
    {
        return;
    }

    //  逐位扫描，发现有置位的则说明按键按下，进行进一步处理
    gpio_input_data &= 0xffff;
    for (uint8_t col_inx = 0; col_inx < MX_COL_COUNT; col_inx++)
    {
        if ((gpio_input_data & 0x0001) == 0x0001)
        {
            //  这里得到的都是以GPIOA_x为行，GPIOB__x为列的原始键值，通过该函数进一步处理
            handle_original_code(row_inx, col_inx);
        }

        //  数据位移，为扫描下一位做准备，如果位移后数据为0，则直接退出
        gpio_input_data >>= 1;
        if (gpio_input_data == 0)
        {
            break;
        }
    }
}

/******************************************** 处理得出的物理键码数据 ***********************************************/

static void handle_original_code(uint8_t row_code, uint8_t col_code)
{
    //  判断该键是否与已按下的按键冲突
    if (is_ghosting(row_code - ROW_OFFSET, col_code) == FALSE)
    {
        //  在模拟的矩阵中，将已按下的按键置位，方便检测按键冲突
        gs_mx_input_key_buffer[row_code - ROW_OFFSET][col_code] = 1;
        gs_mx_input_key_buffer_count++;

        /* 得出HID键码 */
        uint8_t key_code = gs_phy_to_keycode[gs_phy_mx[row_code - ROW_OFFSET][col_code]];
 
        //  判断是否为ctrl、shift等特殊按键，并将按键加入到数据缓冲区中
        uint8_t key_code_row = (key_code >> 4);
        uint8_t key_code_col = (key_code & 0x0f);
        if (key_code_row == 0x0e)
        {
            gs_temp_key_buffer.buffer[0] |= (0x01 << key_code_col);
        }
        else
        {
            gs_temp_key_buffer.buffer[gs_temp_key_buffer.normal_key_count + 2] = key_code;
            gs_temp_key_buffer.normal_key_count += 1;
        }
        gs_temp_key_buffer.key_count++;
    }
    else
    {
        gs_ghosting_flag = TRUE;
    }
}

/* 判断是否有按键冲突 */
static bool is_ghosting(uint8_t row_code, uint8_t col_code)
{
    if (gs_mx_input_key_buffer_count < 2)
    {
        return FALSE;
    }

    if (gs_temp_key_buffer.key_count > 14 || gs_temp_key_buffer.normal_key_count > 6)
    {
        return TRUE;
    }

    //  判断是否有同列
    for (uint8_t i = 0; i < row_code; i++)
    {
        if (gs_mx_input_key_buffer[i][col_code] == 1)
        {
            for (uint8_t j = 0; j < MX_COL_COUNT; j++)
            {
                if (j == col_code)
                {
                    continue;
                }
                if ((gs_mx_input_key_buffer[i][j] == 1) || (gs_mx_input_key_buffer[row_code][j] == 1))
                {
                    return TRUE;
                }
            }
        }
    }

    //  判断是否有同行
    for (uint8_t i = 0; i < col_code; i++)
    {
        if (gs_mx_input_key_buffer[row_code][i] == 1)
        {
            for (uint8_t j = 0; j < row_code; j++)
            {
                if ((gs_mx_input_key_buffer[j][i] == 1) || (gs_mx_input_key_buffer[j][col_code] == 1))
                {
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

/********************************************** 工具函数 **************************************************/

static uint16_t get_col_data(void)
{
    uint16_t col_data = gpio_input_port_get(GPIOB);

    return col_data;
}
