#include "gd32f3x0.h"
#include "key_gpio.h"

/*
    Pb 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 (输入)
    Pa
     0
     1
     2
     3
     4
     5
     6
   7(E)
   (输入)
*/

void key_gpio_config(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    gpio_deinit(GPIOA);
    gpio_deinit(GPIOB);

    gpio_mode_set(
        GPIOA,
        GPIO_MODE_OUTPUT,
        GPIO_PUPD_NONE,
        GPIO_PIN_8 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    gpio_output_options_set(
        GPIOA,
        GPIO_OTYPE_PP,
        GPIO_OSPEED_50MHZ,
        GPIO_PIN_8 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);

    gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_ALL);
}
