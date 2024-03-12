#ifndef USB_USER_H
#define USB_USER_H

#include <stdint.h>

void init_usb(void);
void set_key_buffer(uint8_t inx, uint8_t byte);
uint8_t get_key_buffer_byte(uint8_t inx);
uint8_t* get_key_buffer(void);
uint8_t find_key_buffer(uint8_t key_code);
uint8_t buffer_cmp(uint8_t *temp_buffer);

#endif // USB_USER_H
