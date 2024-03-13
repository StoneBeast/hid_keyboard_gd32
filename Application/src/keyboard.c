#include "keyboard.h"
#include "key_gpio.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GPIO_PIN(x) BIT(x)

static uint8_t temp_key_buffer[8] = {0};
static uint8_t normal_key_count = 0;
extern usb_core_handle_struct usbhs_core_dev;
static uint8_t ghosting_flag = 0;

static uint8_t original_key_code_arr[15] = {0};
static uint8_t original_key_count = 0;

static uint8_t add_to_original_key(uint8_t orig_key_code);
static void reset_original_key(void);
static void add_to_temp_key_buffer(uint8_t key_code);
static uint8_t is_ghosting(uint8_t key_code);
static void handle_input_key(uint8_t row, uint8_t col);
static void handle_keyboard_input(uint8_t row_inx, uint16_t col_data);

/*!
    \brief      scan the keyboard matrix
    \param[in]  none
    \param[out] none
    \retval     none
*/
void scan_keyboard(void)
{

    uint16_t col_data = 0x0000;

    //  GPIOA_PINx(1..8) set
    gpio_port_write(GPIOA, (gpio_output_port_get(GPIOA) | 0x01fe));

    while (1)
    {
        uint8_t row = 0;
        delay_ms(20);
        //  reset Special key
        for (row = 1; row <= 8; row++)
        {
            //  scan rowx(1..8)
            gpio_bit_reset(GPIOA, GPIO_PIN(row));

            col_data = gpio_input_port_get(GPIOB);
            handle_keyboard_input(row, col_data);

            //  set current row for scanning next row
            gpio_bit_set(GPIOA, GPIO_PIN(row));
        }

        //  产生冲突之后，松开直到所有按键才能恢复键盘功能
        if (normal_key_count == 0)
        {
            ghosting_flag = 0;
            reset_original_key();
        }

        //  if buffer changed, send new one
        //  只有在按键发生变化之后才会修改key buffer，并发送key buffer
        //  只有在最后发送时，使用的才是key buffer,而修改等操作都是在temp_key_buffer上进行的。
        if (buffer_cmp(temp_key_buffer) == 0)
        {
            // debug_port_num_code(0x00, GPIO_PIN_6);
            if (ghosting_flag)
            {
                memset(temp_key_buffer, 0, 8);
            }
            else
            {
                reset_original_key();
            }

            memcpy(get_key_buffer(), temp_key_buffer, 8);

            usbd_hid_report_send(&usbhs_core_dev, get_key_buffer(), 8U);
        }

        //  reset temp_key_buffer and normal_key_count
        memset(temp_key_buffer, 0, 8);
        normal_key_count = 0;
    }
}

static uint8_t add_to_original_key(uint8_t orig_key_code)
{
    if (original_key_count < 12)
    {
        original_key_code_arr[original_key_count] = orig_key_code;
        original_key_count++;

        return 1;
    }
    else 
    {
        return 0;
    }

}

static void reset_original_key(void)
{
    memset(original_key_code_arr, 0, sizeof(original_key_code_arr));
    original_key_count = 0;
}

static void add_to_temp_key_buffer(uint8_t key_code)
{
    if ((key_code>>4) == 0xe)
    {
        temp_key_buffer[0] = (temp_key_buffer[0] | (0x01 << (key_code & 0x0f)));
        add_to_original_key((key_code&0x0f) | 0x70);
    }
    else 
    {
        uint8_t orign_key_code = key_code;
        temp_key_buffer[normal_key_count+2] = key_code;
        if ((key_code>>4) == 0)
        {
            orign_key_code = key_code | 0x80;
        }

        normal_key_count++;
        add_to_original_key(orign_key_code);
    }
}

//  使用减法，然后判断差的绝对值，是否符合，来判断是否构成冲突的条件
//  1y/0n
static uint8_t is_ghosting(uint8_t key_code)
{
    if (ghosting_flag)
    {
        normal_key_count ++;
        return 1;
    }
    
    if (normal_key_count < 2)
    {
        return 0;
    }
    else
    {
        //  获得物理按键码
        if ((key_code&0xf0) == 0x00)
        {
            key_code |= 0x80;
        }
        else if ((key_code&0xf0) == 0xe0)
        {
            key_code &= 0x0f;
            key_code |= 0x70;
        }

        // uint8_t result = 0;
        // uint8_t w_count = 0;

        /*
            由于扫描顺序的原因，可能出现非法键的位置只可能是矩形的下面一行，
            因此首先需要判断已按下的按键中是否有同列的，如果有同列的，再判断这个同列
            的按键是否有同行按键即可，注意：可能不止有一个同列按键，因此每一个都需要检查。
        */
        uint8_t col_code = (key_code & 0xf0);
        
        for (uint8_t i = 0; i < original_key_count; i++)
        {
            if ((original_key_code_arr[i] & col_code) == col_code)
            {
                // 发现同列按键, 判断该按键是否有同行按键
                for (uint8_t j = 0; j < original_key_count; j++)
                {
                    if ((original_key_code_arr[i]^original_key_code_arr[j]) < 0x10)
                    {
                        return 1;
                    }
                }
                

            }
        }
        return 0;
    }
}

//  扫描整个矩阵之后再统一发送
/*!
    \brief      get key code with row and col
    \param[in]  row: row code
    \param[in]  col: col code
    \param[out] none
    \retval     none
*/
static void handle_input_key(uint8_t row, uint8_t col)
{
    uint8_t key_code = 0x00;
    if (row == 0x07)
    {
        key_code = ( 0xe0 | col );

        // if (normal_key_count > 2)
        if (is_ghosting(key_code))
        {
            ghosting_flag = 1;
            return;
        }

        add_to_temp_key_buffer(key_code);
    }
    else
    {
        if (row == 8)
        {
            row = 0;
        }
        //  普通按键取前6个，后面的全部丢弃
        //  当前处理方式，会导致多按下的按键代替上一个按键，导致上一个按键被松开，然后当前按键被按下
        //  TODO: 可以在这里控制超过2个，但是并不冲突的按键继续加入buffer
        
        key_code |= (row<<4);
        key_code |= col;


        // if (normal_key_count > 2)
        if (is_ghosting(key_code))
        {
            ghosting_flag = 1;
            return;
        }

        add_to_temp_key_buffer(key_code);
    }
}

/*!
    \brief      handle keyboard input, get col and row then get the key code
    \param[in]  row_inx: row index whitch the key be passed in
    \param[out] col_data: col data when scan one row
    \retval     none
*/
static void handle_keyboard_input(uint8_t row_inx, uint16_t col_data)
{
    if (col_data != 0xffffu) //这里不需要考虑被松开的按键，因为在每轮发送完成后都会清空key buffer
    {
        //  some key be passed
        uint16_t temp_data = ~col_data;
        for (uint8_t i = 1; i <= 16; i++)
        {
            if (temp_data == 0)
            {
                break;
            }

            if (normal_key_count < 6 && ((temp_data >> 1) * 2 != temp_data))
            {
                //  some key in col i-1 be passed
                handle_input_key(row_inx, i-1);
            }
            temp_data >>= 1;
        }
    }
}

