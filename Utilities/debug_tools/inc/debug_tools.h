#ifndef DEBUG_TOOLS_H
#define DEBUG_TOOLS_H

#include "gd32f3x0.h"
#include <stdio.h>
#include "usb_delay.h"

void usart0_gpio_config(void);
void usart0_config(void);
void test_exti(void);

void debug_port_init(uint32_t GPIOF_PIN);
void debug_port_num(uint8_t n);
void debug_port_code(uint8_t x, uint8_t y);
void debug_port_byte(uint8_t *bp, uint8_t len);
void debug_soft_uart_TX_init(void);
void debug_soft_uart_send_data(uint8_t data);
void debug_test_gpio_output(uint32_t gpio_periph, uint32_t pin);
void debug_port_num_code_hw(uint16_t data);
void debug_port_num_code_by(uint8_t data);
#endif // DEBUG_TOOLS_H
