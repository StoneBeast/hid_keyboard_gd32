#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include "gd32f3x0.h"
#include <stdio.h>
#include "usb_delay.h"

void usart0_gpio_config(void);
void usart0_config(void);
void test_exti(void);

void debug_port_init(uint32_t GPIOF_PIN);
void debug_port_num(uint8_t n, uint32_t GPIOF_PIN);
void debug_port_code(uint8_t x, uint8_t y, uint32_t GPIOF_PIN);
void debug_port_byte(uint8_t *bp, uint8_t len, uint32_t GPIOF_PIN);
void debug_soft_uart_TX_init(uint32_t GPIOF_PIN);
void debug_soft_uart_send_data(uint8_t data, uint32_t GPIOF_PIN);
void debug_test_gpio_output(uint32_t gpio_periph, uint32_t pin);
void debug_port_num_code(uint8_t data, uint32_t GPIOF_PIN);
#endif // DEBUG_TOOLS_H
