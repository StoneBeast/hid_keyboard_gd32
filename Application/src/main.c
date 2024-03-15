#include "usb_user.h"
#include "keyboard.h"
#include "debug_tools.h"


#define IS_TEST 1

/*!
    \brief      main routine will construct a USB keyboard
    \param[in]  none
    \param[out] none
    \retval     none
*/
int main(void)
{
    init_usb();
    
    debug_port_init(GPIO_PIN_7);
    // debug_port_num_code(0xab, GPIO_PIN_6);
    scan_keyboard();
   
    while (1);   
}

