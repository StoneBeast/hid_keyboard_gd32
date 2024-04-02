#include "gd32f3x0.h"
#include "key_gpio.h"

void key_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOF);

    gpio_mode_set(
        GPIOA,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE,
        GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

    gpio_output_options_set(
        GPIOA,
        GPIO_OTYPE_PP,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10);

    gpio_mode_set(
        GPIOA,
        GPIO_MODE_INPUT,
        GPIO_PUPD_PULLDOWN,
        GPIO_PIN_0 | GPIO_PIN_1);
    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);

    gpio_mode_set(
        GPIOF,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE,
        GPIO_PIN_6 | GPIO_PIN_7);

    gpio_output_options_set(
        GPIOF,
        GPIO_OTYPE_PP,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_6 | GPIO_PIN_7);
}
