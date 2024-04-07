#ifndef USB_USER_H
#define USB_USER_H

#include <stdint.h>

#define LED_NUM_LOCK_POS 0
#define LED_CAP_LOCK_POS 1
#define LED_SCR_LOCK_POS 2

#define LED_NUM_LOCK_MASK (0X01 << LED_NUM_LOCK_POS)
#define LED_CAP_LOCK_MASK (0X01 << LED_CAP_LOCK_POS)
#define LED_SCR_LOCK_MASK (0X01 << LED_SCR_LOCK_POS)

void init_usb(void);
void set_key_buffer(uint8_t inx, uint8_t byte);
uint8_t get_key_buffer_byte(uint8_t inx);
uint8_t* get_key_buffer(void);
uint8_t buffer_cmp(uint8_t *temp_buffer);
uint8_t find_buffer(const uint8_t *buffer, uint8_t key_code);
void handle_led_gpio(uint8_t status_data);

#endif // USB_USER_H
