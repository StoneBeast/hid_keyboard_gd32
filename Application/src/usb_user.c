#include "hid_core.h"
#include "usb_delay.h"
#include "usb_user.h"
#include "key_gpio.h"

// static uint8_t key_buffer[HID_IN_PACKET] = {0};
uint8_t key_buffer[HID_IN_PACKET] = {0};

__IO uint8_t prev_transfer_complete = 1U;

uint8_t timer_prescaler = 0;
uint32_t usbfs_prescaler = 0;

usb_core_handle_struct usbhs_core_dev =
    {
        .dev =
            {
                .dev_desc = (uint8_t *)&device_descripter,
                .config_desc = (uint8_t *)&configuration_descriptor,
                .strings = usbd_strings,
                .class_init = usbd_hid_init,
                .class_deinit = usbd_hid_deinit,
                .class_req_handler = usbd_hid_classreq_handle,
                .class_data_handler = usbd_hid_data_handler,
            },

        .udelay = delay_us,
        .mdelay = delay_ms};

/*!
    \brief      configure USB clock
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void usb_clock_config(void)
{
    uint32_t system_clock = rcu_clock_freq_get(CK_SYS);

    if (system_clock == 48000000)
    {
        usbfs_prescaler = RCU_USBFS_CKPLL_DIV1;
        timer_prescaler = 3;
    }
    else if (system_clock == 72000000)
    {
        usbfs_prescaler = RCU_USBFS_CKPLL_DIV1_5;
        timer_prescaler = 5;
    }
    else if (system_clock == 96000000)
    {
        usbfs_prescaler = RCU_USBFS_CKPLL_DIV2;
        timer_prescaler = 7;
    }
    else
    {
        /*  reserved  */
    }

#ifndef USE_IRC48M
    rcu_ck48m_clock_config(RCU_CK48MSRC_PLL48M);

    rcu_usbfs_clock_config(usbfs_prescaler);
#else
    /* enable IRC48M clock */
    rcu_osci_on(RCU_IRC48M);

    /* wait till IRC48M is ready */
    while (SUCCESS != rcu_osci_stab_wait(RCU_IRC48M))
    {
    }

    rcu_ck48m_clock_config(RCU_CK48MSRC_IRC48M);
#endif /* USE_IRC48M */

    rcu_periph_clock_enable(RCU_USBFS);
}

/*!
    \brief      configure USB interrupt
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void usb_interrupt_config(void)
{
    nvic_irq_enable((uint8_t)USBFS_IRQn, 3U, 0U);

    /* enable the power module clock */
    rcu_periph_clock_enable(RCU_PMU);

    /* USB wakeup EXTI line configuration */
    exti_interrupt_flag_clear(EXTI_18);
    exti_init(EXTI_18, EXTI_INTERRUPT, EXTI_TRIG_RISING);
    exti_interrupt_enable(EXTI_18);

    nvic_irq_enable((uint8_t)USBFS_WKUP_IRQn, 0U, 0U);
}

#ifdef USE_IRC48M

/*!
    \brief      configure the CTC peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void ctc_config(void)
{
    /* config CTC reference signal source prescaler */
    ctc_refsource_prescaler_config(CTC_REFSOURCE_PSC_OFF);
    /* select reference signal source */
    ctc_refsource_signal_select(CTC_REFSOURCE_USBSOF);
    /* select reference signal source polarity */
    ctc_refsource_polarity_config(CTC_REFSOURCE_POLARITY_RISING);
    /* config hardware automatically trim mode */
    ctc_hardware_trim_mode_config(CTC_HARDWARE_TRIM_MODE_ENABLE);

    /* config CTC counter reload value, Fclock/Fref-1 */
    ctc_counter_reload_value_config(0xBB7F);
    /* config clock trim base limit value, Fclock/Fref*0.0012/2 */
    ctc_clock_limit_value_config(0x1D);

    /* CTC counter enable */
    ctc_counter_enable();
}

#endif

/*!
    \brief      init usb
    \param[in]  none
    \param[out] none
    \retval     none
*/
void init_usb(void)
{
    /* configure USB clock */
    usb_clock_config();

    /* timer nvic initialization */
    timer_nvic_init();

    /* key gpio  initialization*/
    key_gpio_config();

    /* USB device stack configure */
    usbd_init(&usbhs_core_dev, USB_FS_CORE_ID);

    /* USB interrupt configure */
    usb_interrupt_config();

#ifdef USE_IRC48M
    /* CTC peripheral clock enable */
    rcu_periph_clock_enable(RCU_CTC);

    /* CTC config */
    ctc_config();

    while (ctc_flag_get(CTC_FLAG_CKOK) == RESET);
#endif

    /* check if USB device is enumerated successfully */
    while (usbhs_core_dev.dev.status != USB_STATUS_CONFIGURED);
}

/*!
    \brief      key buffer byte setter
    \param[in]  inx:  index of byte in key_buffer
    \param[in]  byte: new byte
    \retval     none
*/
void set_key_buffer(uint8_t inx, uint8_t byte)
{
    key_buffer[inx] = byte;
}

/*!
    \brief      key buffer byte getter
    \param[in]  inx:  index of byte in key_buffer
    \retval     target byte
*/
uint8_t get_key_buffer_byte(uint8_t inx)
{
    return key_buffer[inx];
}

/*!
    \brief      key buffer getter
    \param[in]  none
    \retval     pointer of key buffer
*/
uint8_t* get_key_buffer(void)
{
    return key_buffer;
}

/*!
    \brief      find key code in the key buffer
    \param[in]  key_code: key code to find
    \retval     index of key code in the key buffer or 0
*/
uint8_t find_key_buffer(uint8_t key_code)
{
    for (uint8_t i=2; i<8; i++)
    {
        if (key_buffer[i] == key_code)
        {
            return i;
        }
    }
    return 0;
}

/*!
    \brief      compare current key buffer and new key buffer
    \param[in]  temp_buffer: pointer of new buffer
    \retval     0/1: different or same with current key buffer
*/
uint8_t buffer_cmp(uint8_t * temp_buffer)
{
    uint8_t eq_flag = 1;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (key_buffer[i] != temp_buffer[i])
        {
            eq_flag = 0;
            break;
        }
    }

    return eq_flag;
    
}
