#include "debug_tools.h"

#define DEBUG_PORT GPIOF
#define DEBUG_RCU_PORT RCU_GPIOF

static uint32_t delayTime = 99; 
static uint32_t debug_pin;

/*!
    \brief      cofigure the USART0 GPIO ports
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usart0_gpio_config(void)
{
    /* enable COM GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOB);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_6);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOB, GPIO_AF_0, GPIO_PIN_7);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_6);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_6);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_7);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_7);
}

/*!
    \brief      cofigure the USART0
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usart0_config(void)
{
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* USART configure */
    usart_deinit(USART0);
    usart_word_length_set(USART0, USART_WL_8BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);
    usart_parity_config(USART0, USART_PM_NONE);
    usart_baudrate_set(USART0, 115200U);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);

    usart_enable(USART0);
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART0, (uint8_t) ch);
    while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
    return ch;
}

void test_exti(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_1);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_0);

    nvic_irq_enable(EXTI0_1_IRQn, 2u, 0u);

    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN1);
    syscfg_exti_line_config(EXTI_SOURCE_GPIOB, EXTI_SOURCE_PIN0);

    exti_deinit();

    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
    exti_init(EXTI_1, EXTI_INTERRUPT, EXTI_TRIG_BOTH);

    exti_interrupt_flag_clear(EXTI_0);
    exti_interrupt_flag_clear(EXTI_1);
}

void debug_port_init(uint32_t GPIOF_PIN)
{
    debug_pin = GPIOF_PIN;

    rcu_periph_clock_enable(DEBUG_RCU_PORT);
    gpio_deinit(DEBUG_PORT);
    gpio_mode_set(DEBUG_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIOF_PIN);
    gpio_output_options_set(DEBUG_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIOF_PIN);
    gpio_bit_reset(DEBUG_PORT, GPIOF_PIN);
}

void debug_port_num(uint8_t n)
{
    // uint32_t time = 0;
    uint8_t temp = n+1;

    // while (time++ < 200000);
    delay_ms(50);
    while (temp>0)
    {
        gpio_bit_set(DEBUG_PORT, debug_pin);
        gpio_bit_reset(DEBUG_PORT, debug_pin);
        temp--;
    }
}

void debug_port_num_code_by(uint8_t data)
{
    // uint8_t p = data >> 4;

    // debug_port_num(p, GPIOF_PIN);
    // delay_ms(5);
    // debug_port_num(data & 0x0f, GPIOF_PIN);
    delay_us(150);
    for (uint8_t i = 0; i < 8; i++)
    {
        gpio_bit_set(DEBUG_PORT, debug_pin);
        delay_us(50);
        gpio_bit_write(DEBUG_PORT, debug_pin, (data >> i) & 0x01);
        gpio_bit_set(DEBUG_PORT, debug_pin);
        delay_us(50);
        gpio_bit_reset(DEBUG_PORT, debug_pin);
        delay_us(50);
    }
}

void debug_port_num_code_hw(uint16_t data)
{
    // uint8_t p = data >> 4;

    // debug_port_num(p, GPIOF_PIN);
    // delay_ms(5);
    // debug_port_num(data & 0x0f, GPIOF_PIN);
    delay_us(150);
    for (uint8_t i = 0; i < 16; i++)
    {
        gpio_bit_set(DEBUG_PORT, debug_pin);
        delay_us(50);
        gpio_bit_write(DEBUG_PORT, debug_pin, (data >> i) & 0x01);
        gpio_bit_set(DEBUG_PORT, debug_pin);
        delay_us(50);
        gpio_bit_reset(DEBUG_PORT, debug_pin);
        delay_us(50);
    }
}

void debug_port_code(uint8_t x, uint8_t y)
{
    // uint32_t time = 0;
    uint8_t temp_x = x+1;
    uint8_t temp_y = y+1;

    // while (time++ < 200000);
    delay_ms(100);
    while (temp_x>0)
    {
        gpio_bit_set(DEBUG_PORT, debug_pin);
        gpio_bit_reset(DEBUG_PORT, debug_pin);
        temp_x--;
    }

    // while (time-- > 185000);
    delay_ms(10);

    while (temp_y > 0)
    {
        gpio_bit_set(DEBUG_PORT, debug_pin);
        gpio_bit_reset(DEBUG_PORT, debug_pin);
        temp_y--;
    }
}


void debug_soft_uart_TX_init()
{
    rcu_periph_clock_enable(DEBUG_RCU_PORT);
    gpio_mode_set(DEBUG_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLUP, debug_pin);
    gpio_output_options_set(DEBUG_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, debug_pin);

    gpio_bit_set(DEBUG_PORT, debug_pin);
}

/*!
 * @brief 	模拟串口1发送一个字节
 * @param
 * @return	none
 * @note	数据低位在前高位在后
 */
void debug_soft_uart_send_data(uint8_t data)
{
    // TODO:没有实现功能，有可能是delay_us有问题
    uint8_t i = 0;
    gpio_bit_reset(DEBUG_PORT, debug_pin); //!< 起始位
    delay_us(delayTime);
    for (i = 0; i < 8; i++)
    {
        if (data & 0x01)
            gpio_bit_set(DEBUG_PORT, debug_pin);
        else
            gpio_bit_reset(DEBUG_PORT, debug_pin);
        delay_us(delayTime);
        data >>= 1;
    }
    gpio_bit_set(DEBUG_PORT, debug_pin); //!< 停止位
    delay_us(delayTime);
}

void debug_test_gpio_output(uint32_t gpio_periph, uint32_t pin)
{
    switch (gpio_periph)
    {
    case GPIOA:
        rcu_periph_clock_enable(RCU_GPIOA);
        break;
    case GPIOB:
        rcu_periph_clock_enable(RCU_GPIOB);
        break;
    case GPIOC:
        rcu_periph_clock_enable(RCU_GPIOC);
        break;
    case GPIOD:
        rcu_periph_clock_enable(RCU_GPIOD);
        break;
    case GPIOF:
        rcu_periph_clock_enable(RCU_GPIOF);
        break;

    default:
        break;
    }
    gpio_mode_set(gpio_periph, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, pin);

    gpio_output_options_set(gpio_periph, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, pin);

    gpio_bit_set(gpio_periph, pin);

    while (1)
    {
        uint32_t time = 0;
        while (time++ < 20000);
        gpio_bit_toggle(gpio_periph, pin);
    }
}

