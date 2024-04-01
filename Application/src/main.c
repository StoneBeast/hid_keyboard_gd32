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
    
    scan_keyboard();
   
    while (1);   
}

