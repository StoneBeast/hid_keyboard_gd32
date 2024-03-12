/*
    使用逐行扫描法，(这里使所有接收引脚设为上拉输入，所有输出引脚默认输出为高电平)
    即每行输出逐一输出低电平，然后读取所有输入引脚的数据，若发现有低电平的，则说明该行该列有被按下的按键。
*/

#include "keyboard.h"
#include "key_gpio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GPIO_PIN(x) BIT(x)

typedef struct
{
    uint8_t buffer[14];
    uint8_t key_count;
} buffer_t;

static buffer_t gs_temp_input_key_buffer = {.buffer = {0}, .key_count = 0};
//  接收输入按键，用于验证合法性以及转化为key code
static buffer_t gs_input_key_buffer = {.buffer = {0}, .key_count = 0};
//  作为实际发送的key buffer的缓冲
static buffer_t gs_temp_key_buffer = {.buffer = {0}, .key_count = 0};

static void handle_input_data(uint8_t row_inx, uint16_t gpio_input_data);
static void handle_original_code(uint8_t row_code, uint8_t col_code);

/*!
    \brief      scan the keyboard matrix
    \param[in]  none
    \param[out] none
    \retval     none
*/
void scan_keyboard(void)
{
    while (1)
    {
        uint16_t col_data = 0x0000;
        for (uint8_t row_inx = 1; row_inx < 9; row_inx++)
        {
            //  逐行扫描
            gpio_port_write(GPIOA, 0x0001 >> row_inx);
            //  获取当前的col输入
            col_data = gpio_input_port_get(GPIOB);
            //  处理col data
            handle_input_data(row_inx, col_data);
        }
    }
}

void handle_input_data(uint8_t row_inx, uint16_t gpio_input_data)
{
    if (gpio_input_data == 0x0000)
    {
        return;
    }

    for (uint8_t col_inx = 0; col_inx <= 15; col_inx++)
    {
        gpio_input_data >>= col_inx;
        if (gpio_input_data == 0)
        {
            break;
        }

        if (gpio_input_data & 0x0001 == 0x0001)
        {
            handle_original_code(row_inx, col_inx);
        }
    }
}

static void handle_original_code(uint8_t row_code, uint8_t col_code)
{
}
