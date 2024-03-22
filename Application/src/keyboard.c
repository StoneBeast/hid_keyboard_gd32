#include "keyboard.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "debug_tools.h"

#define BUFFER_SIZE 8
#define GPIO_PIN(x) BIT(x)

static volatile bool gs_ghosting_flag = FALSE;

typedef struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t key_count;
    uint8_t normal_key_count;
} buffer_t;

extern usb_core_handle_struct usbhs_core_dev;

//  接收输入按键，用于验证合法性
static uint8_t gs_mx_input_key_buffer[8][16] = {0};
static uint8_t gs_mx_input_key_buffer_count = 0;

//  作为实际发送的key buffer的缓冲
static buffer_t gs_temp_key_buffer = {.buffer = {0}, .key_count = 0, .normal_key_count = 0};

static uint16_t gs_input_col_data_buffer[16] = {0};

static void handle_input_data(uint8_t row_inx, uint16_t gpio_input_data);
static void handle_original_code(uint8_t row_code, uint8_t col_code);
static bool is_ghosting(uint8_t row_code, uint8_t col_code);

/*!
    \brief      scan the keyboard matrix
    \param[in]  none
    \param[out] none
    \retval     none
*/
void scan_keyboard(void)
{
    gpio_port_write(GPIOA, (gpio_output_port_get(GPIOA) | 0x01fe));

    while (1)
    {
        uint16_t col_data = 0x0000;
        gs_ghosting_flag = FALSE;

        /*
            这里增加判断，当本轮扫描出现冲突时，停止扫描以提高效率，但是每次循环
            开始前，会重置标志位，所以不影响下一次扫描
        */
        for (uint8_t row_inx = 1; (row_inx < 9) && (gs_ghosting_flag==FALSE); row_inx++)
        {
            //  逐行扫描
            gpio_bit_reset(GPIOA, GPIO_PIN(row_inx));

            //  获取当前的col输入
            col_data = gpio_input_port_get(GPIOB);
                //  处理col data
            handle_input_data(row_inx, col_data);

            gpio_bit_set(GPIOA, GPIO_PIN(row_inx));
        }

        if ((gs_ghosting_flag == FALSE) && (buffer_cmp(gs_temp_key_buffer.buffer) == 0))
        {
            memcpy(get_key_buffer(), gs_temp_key_buffer.buffer, 8);
            usbd_hid_report_send(&usbhs_core_dev, get_key_buffer(), 8U);
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
    gpio_input_data = ~gpio_input_data;

    if (gpio_input_data != gs_input_col_data_buffer[row_inx - 1])
    {
        //  消抖
        delay_ms(50);
        if ((gpio_input_data ^ gpio_input_port_get(GPIOB)) == 0xffff)
        {
            gs_input_col_data_buffer[row_inx - 1] = gpio_input_data;
        }
        else
        {
            gpio_input_data = gs_input_col_data_buffer[row_inx - 1];
        }
    }

    if (gpio_input_data == 0x0000)
    {
        return;
    }

    for (uint8_t col_inx = 0; col_inx <= 15; col_inx++)
    {
        if ((gpio_input_data & 0x0001) == 0x0001)
        {
            handle_original_code(row_inx, col_inx);
        }

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
    if (is_ghosting(row_code-1, col_code) == FALSE)
    {
        gs_mx_input_key_buffer[row_code-1][col_code] = 1;
        gs_mx_input_key_buffer_count++;

        if (row_code == 0x07)
        {
            gs_temp_key_buffer.buffer[0] |= (0x01 << col_code);
        }
        else 
        {
            if (row_code == 0x08)
            {
                row_code = 0x00;
            }

            gs_temp_key_buffer.buffer[gs_temp_key_buffer.normal_key_count+2] = ((row_code << 4) | col_code);
            gs_temp_key_buffer.normal_key_count += 1;
        }
        gs_temp_key_buffer.key_count++;

    }
    else 
    {
        gs_ghosting_flag = TRUE;
    }
}

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
            for(uint8_t j = 0; j < 16; j++)
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
