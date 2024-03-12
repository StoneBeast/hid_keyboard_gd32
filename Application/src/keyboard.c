/*
    使用逐行扫描法，(这里使所有接收引脚设为上拉输入，所有输出引脚默认输出为高电平)
    即每行输出逐一输出低电平，然后读取所有输入引脚的数据，若发现有低电平的，则说明该行该列有被按下的按键。

    原本方案为逐行扫描矩阵，然后对输入分析，填充temp_key_buffer对比后再统一发送扫描结果，
    整体流程：
        1. 扫描一行，扫描结果读取输入时候全为1与否，决定是不是继续向下分析，若不向下分析，则直接开始扫描下一行。
        // 2. 结果不全为1，使用按位取反和位移的方式分析第几位被接通 (时间开销极大) ===更正，时间开销并不大，造成大量时间开销的是防抖延迟
        3. 得出第几位被接通后，与输出的位结合，即可得出按键码
        4. 将按键码按照规则填充至temp_key_buffer中或被舍弃，继续扫描下一行
        5. 所有行被扫描完后，对比当前的key_buffer(即是上一次发送的)和当前的temp_key_buffer(即是本次扫描的结果),若是不同则发送，
           相同则不发送，然后清空temp_key_buffer
    特点：
        1. 决定发不发送数据的因素：temp_key_buffer是否与key_buffer一致(顺序无所谓，案件一致即可),一致则不发送
        2. temp_key_buffer每轮都会被重置，所以即是在新一轮扫描中不按下任何按键，而在上一轮中按下了，仍然可以被检测到，同时时间开销极小
        // 3. 由于判断问题，导致即使被按下的按键没有发生改变，但只要按下按键，就会导致每轮扫描的时间开销极大(5us->100ms)，时间间隔增大到了
        //    人可以感知的程度，而且随着按键的增加，这个时间间隔会不断增大，最终到达完全不可用的程度。 ===更正，详见2
        4. 在同一个扫描循环中按下的按键，在扫描中会丧失原本的顺序，按照扫描顺序，但是当扫描间隔缩小到一定程度之后，这个顺序就可以忽略不计了。
        5. 按键冲突：当按下特定的三个按键之后，第四个按键也会被按下，这里可以将第3、4两个按键理解为同时按下，那么判断也无法判断了，
           也就是说不存在通过判断按下顺序，保留第三个舍弃第四个。
    改进思路：
        1. 进一步限制进入第2步的条件，可以进一步提升效率
        2. 在没有特殊电路的情况下，处理按键冲突只能采取舍弃的措施。即最高只能同时读取两个按键。
        3. 采取判断方式，区分第三个按下的是不是特殊按键，如果不是可以继续读取？
*/

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

        uint8_t result = 0;
        uint8_t w_count = 0;
#if 1
        for (uint8_t i = 0; i < original_key_count; i++)
        {
            result = (original_key_code_arr[i] < key_code ? key_code - original_key_code_arr[i] : original_key_code_arr[i] - key_code);

            if (result == 0x01 || result == 0x10 || result == 0x11 || result == 0x0f)
            {
                w_count ++;
                if (w_count >= 2)
                {
                    return 1;
                }
            }
        }
#else   // 0
        for (uint8_t i = 0; i < original_key_count; i++)
        {
            //  判断是否存在同行按键

        }
#endif  // 0
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

