/*!
    \file  hid_core.c
    \brief USB HID device class core driver

    \version 2017-06-06, V1.0.0, firmware for GD32F3x0
    \version 2019-06-01, V2.0.0, firmware for GD32F3x0
*/

/*
    Copyright (c) 2019, GigaDevice Semiconductor Inc.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "hid_core.h"
#include "usbd_std.h"
#include "usbd_int.h"

static uint32_t usbd_hid_altset = 0U;
static uint32_t usbd_hid_protocol = 0U;
static uint32_t usbd_hid_idlestate  = 0U;
static uint8_t  usbd_hid_report_buffer[2] = {0};

extern __IO uint8_t prev_transfer_complete;
// extern void set_key_buffer(uint8_t inx, uint8_t byte);
extern uint8_t get_key_buffer_byte(uint8_t inx);
extern void led_handler(uint8_t data_fragment);

usbd_int_cb_struct *usbd_int_fops = NULL;

#define USBD_VID                     0x28E9
#define USBD_PID                     0x0380

/* Note:it should use the C99 standard when compiling the below codes */
/* USB standard device descriptor */
__ALIGN_BEGIN usb_descriptor_device_struct device_descripter __ALIGN_END =
{
    .Header = 
     {
         .bLength = USB_DEVICE_DESC_SIZE, 
         .bDescriptorType = USB_DESCTYPE_DEVICE
     },
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = USB_MAX_EP0_SIZE,
    .idVendor = USBD_VID,
    .idProduct = USBD_PID,
    .bcdDevice = 0x0200,
    .iManufacturer = USBD_MFC_STR_IDX,
    .iProduct = USBD_PRODUCT_STR_IDX,
    .iSerialNumber = USBD_SERIAL_STR_IDX,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

__ALIGN_BEGIN usb_descriptor_configuration_set_struct configuration_descriptor __ALIGN_END =
    {
        .config =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_descriptor_configuration_struct),
                        .bDescriptorType = USB_DESCTYPE_CONFIGURATION},
                .wTotalLength = USB_HID_CONFIG_DESC_SIZE,
                .bNumInterfaces = 0x02,
                .bConfigurationValue = 0x01,
                .iConfiguration = 0x00,
                .bmAttributes = 0xA0,
                .bMaxPower = 0x32},

        .hid_interface =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_descriptor_interface_struct),
                        .bDescriptorType = USB_DESCTYPE_INTERFACE},
                .bInterfaceNumber = 0x00,
                .bAlternateSetting = 0x00,
                .bNumEndpoints = 0x01,
                .bInterfaceClass = 0x03,
                .bInterfaceSubClass = 0x01,
                .bInterfaceProtocol = 0x01,
                .iInterface = 0x00},

        .hid_vendor =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_hid_descriptor_hid_struct),
                        .bDescriptorType = HID_DESC_TYPE},
                .bcdHID = 0x0111,
                .bCountryCode = 0x00,
                .bNumDescriptors = 0x01,
                .bDescriptorType = HID_REPORT_DESCTYPE,
                .wDescriptorLength = USB_HID_REPORT_DESC_SIZE,
            },

        .hid_in_endpoint =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_descriptor_endpoint_struct),
                        .bDescriptorType = USB_DESCTYPE_ENDPOINT},
                .bEndpointAddress = HID_IN_EP,
                .bmAttributes = 0x03,
                .wMaxPacketSize = HID_IN_PACKET,
                .bInterval = 18},

        .hid_fn_interface =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_descriptor_interface_struct),
                        .bDescriptorType = USB_DESCTYPE_INTERFACE},
                .bInterfaceNumber = 0x01,
                .bAlternateSetting = 0x00,
                .bNumEndpoints = 0x01,
                .bInterfaceClass = 0x03,
                .bInterfaceSubClass = 0x00,
                .bInterfaceProtocol = 0x00,
                .iInterface = 0x00},

        .hid_fn_custom =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_hid_descriptor_hid_struct),
                        .bDescriptorType = HID_DESC_TYPE},
                .bcdHID = 0x0111,
                .bCountryCode = 0x00,
                .bNumDescriptors = 0x01,
                .bDescriptorType = HID_REPORT_DESCTYPE,
                .wDescriptorLength = USB_HID_FN_REPORT_DESC_SIZE,
            },

        .hid_fn_endpoint =
            {
                .Header =
                    {
                        .bLength = sizeof(usb_descriptor_endpoint_struct),
                        .bDescriptorType = USB_DESCTYPE_ENDPOINT},
                .bEndpointAddress = HID_FN_IN_EP,
                .bmAttributes = 0x03,
                .wMaxPacketSize = HID_IN_PACKET,
                .bInterval = 0x30}};

/* USB language ID Descriptor */
__ALIGN_BEGIN usb_descriptor_language_id_struct usbd_language_id_desc __ALIGN_END = 
{
    .Header = 
     {
         .bLength = sizeof(usb_descriptor_language_id_struct), 
         .bDescriptorType = USB_DESCTYPE_STRING
     },
    .wLANGID = ENG_LANGID
};

__ALIGN_BEGIN uint8_t* usbd_strings[] __ALIGN_END = 
{
    [USBD_LANGID_STR_IDX] = (uint8_t *)&usbd_language_id_desc,
    [USBD_MFC_STR_IDX] = USBD_STRING_DESC("GigaDevice"),
    [USBD_PRODUCT_STR_IDX] = USBD_STRING_DESC("GD32 USB Keyboard in FS Mode"),
    [USBD_SERIAL_STR_IDX] = USBD_STRING_DESC("GD32F3X0-V1.0.0-3a4b5ec")
};

__ALIGN_BEGIN uint8_t hid_report_desc[USB_HID_REPORT_DESC_SIZE] __ALIGN_END =
    {
        0x05, 0x01,       // USAGE_PAGE (Generic Desktop)
        0x09, 0x06,       // USAGE (Keyboard)
        0xa1, 0x01,       // COLLECTION (Application)
        0x05, 0x07,       //   USAGE_PAGE (Keyboard)
        0x19, 0xe0,       //   USAGE_MINIMUM (Keyboard LeftControl)
        0x29, 0xe7,       //   USAGE_MAXIMUM (Keyboard Right GUI)
        0x15, 0x00,       //   LOGICAL_MINIMUM (0)
        0x25, 0x01,       //   LOGICAL_MAXIMUM (1)
        0x75, 0x01,       //   REPORT_SIZE (1)
        0x95, 0x08,       //   REPORT_COUNT (8)
        0x81, 0x02,       //   INPUT (Data,Var,Abs)
        0x95, 0x01,       //   REPORT_COUNT (1)
        0x75, 0x08,       //   REPORT_SIZE (8)
        0x81, 0x01,       //   INPUT (Cnst,Ary,Abs)
        0x95, 0x03,       //   REPORT_COUNT (3)
        0x75, 0x01,       //   REPORT_SIZE (1)
        0x05, 0x08,       //   USAGE_PAGE (LEDs)
        0x19, 0x01,       //   USAGE_MINIMUM (Num Lock)
        0x29, 0x03,       //   USAGE_MAXIMUM (Scroll Lock)
        0x91, 0x02,       //   OUTPUT (Data,Var,Abs)
        0x95, 0x01,       //   REPORT_COUNT (1)
        0x75, 0x05,       //   REPORT_SIZE (5)
        0x91, 0x01,       //   OUTPUT (Cnst,Ary,Abs)
        0x95, 0x06,       //   REPORT_COUNT (6)
        0x75, 0x08,       //   REPORT_SIZE (8)
        0x15, 0x00,       //   LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00, //   LOGICAL_MAXIMUM (255)
        0x05, 0x07,       //   USAGE_PAGE (Keyboard)
        0x19, 0x00,       // USAGE_MINIMUM (Reserved (no event indicated))
        0x29, 0x65,       // USAGE_MAXIMUM (Keyboard Application)
        0x81, 0x00,       // INPUT (Data,Ary,Abs)
        0xc0              //               END_COLLECTION
};

__ALIGN_BEGIN uint8_t hid_fn_report_desc[USB_HID_FN_REPORT_DESC_SIZE] __ALIGN_END =
    {
        0x05, 0x0c,                   // USAGE_PAGE (Consumer Devices)
        0x09, 0x01,                   // USAGE (Consumer Control)
        0xa1, 0x01,                   // COLLECTION (Application)
        0x85, 0x02,                   //   Report ID (2)
        0x75, 0x01,                   //   REPORT_SIZE (1)
        /* 使用的位，以 */
        0x95, 0x06,                   //   REPORT_COUNT (6)
        0x05, 0x0c,                   //   USAGE_PAGE (Consumer Devices)
        0x09, 0x70,                   //   USAGE (Brightness Decrement)
        0x09, 0x6f,                   //   USAGE (Brightness Increment)
        0x09, 0xea,                   //   USAGE (Volume Down)
        0x09, 0xe9,                   //   USAGE (Volume Up)
        0x09, 0xe2,                   //   USAGE (Mute)
        0x09, 0xCD,                   //   Usage (Play/Pause)
        0x15, 0x00,                   //   LOGICAL_MINIMUM (0)
        0x25, 0x01,                   //   LOGICAL_MAXIMUM (1)
        0x81, 0x02,                   //   INPUT (Data,Var,Abs)

        /* 未使用的位 */
        0x95, 0x02,                   //   REPORT_COUNT (2)
        0x75, 0x01,                   //   REPORT_SIZE (1)
        0x81, 0x01,                   //   INPUT (Cnst,Ary,Abs)
        0xc0                          // END_COLLECTION

        // 35 bytes
};

/*!
    \brief      initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_hid_init (void *pudev, uint8_t config_index)
{
    /* initialize Tx endpoint */
    usbd_ep_init(pudev, &(configuration_descriptor.hid_in_endpoint));
    usbd_ep_init(pudev, &(configuration_descriptor.hid_fn_endpoint));

    return USBD_OK;
}

/*!
    \brief      de-initialize the HID device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_hid_deinit (void *pudev, uint8_t config_index)
{
    /* deinitialize HID endpoints */
    usbd_ep_deinit (pudev, HID_IN_EP);
    usbd_ep_deinit (pudev, HID_FN_IN_EP);

    return USBD_OK;
}

/*!
    \brief      handle the HID class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
uint8_t usbd_hid_classreq_handle (void *pudev, usb_device_req_struct *req)
{
    uint16_t len = 0U;
    uint8_t *pbuf = NULL;

    switch (req->bmRequestType & USB_REQ_MASK) {
    case USB_CLASS_REQ:
        switch (req->bRequest) {
        case GET_REPORT:
            /* no use for this driver */
            break;

        case GET_IDLE:
            usbd_ctltx(pudev, (uint8_t *)&usbd_hid_idlestate, 1U);
            break;

        case GET_PROTOCOL:
            usbd_ctltx(pudev, (uint8_t *)&usbd_hid_protocol, 1U);
            break;

        case SET_REPORT:
            usbd_ctlrx(pudev, usbd_hid_report_buffer, req->wLength);
            break;

        case SET_IDLE:
            usbd_hid_idlestate = (uint8_t)(req->wValue >> 8);
            break;

        case SET_PROTOCOL:
            usbd_hid_protocol = (uint8_t)(req->wValue);
            break;

        default:
            usbd_enum_error (pudev, req);
            return USBD_FAIL; 
        }
        break;
    case USB_STANDARD_REQ:
        /* standard device request */
        switch(req->bRequest) {
        case USBREQ_GET_DESCRIPTOR:
            switch (req->wValue >> 8) {
            case HID_REPORT_DESCTYPE:
            //  根据情况，返回两个不同的报告描述符
                if (req->wValue >> 8 == HID_REPORT_DESCTYPE)
                {
                    if (req->wIndex == 0)
                    {
                        len = USB_MIN(USB_HID_REPORT_DESC_SIZE, req->wLength);
                        pbuf = hid_report_desc;
                    }
                    else
                    {
                        len = USB_MIN(USB_HID_FN_REPORT_DESC_SIZE, req->wLength);
                        pbuf = hid_fn_report_desc;
                    }
                }
                break;
            case HID_DESC_TYPE:
                len = USB_MIN((uint16_t)USB_HID_DESC_SIZE, req->wLength);
                pbuf = (uint8_t *)(&(configuration_descriptor.hid_vendor));
                break;
            default:
                break;
            }
            usbd_ctltx (pudev, pbuf, len);
            break;
        case USBREQ_GET_INTERFACE:
            usbd_ctltx (pudev, (uint8_t *)&usbd_hid_altset, 1U);
            break;
        case USBREQ_SET_INTERFACE:
            usbd_hid_altset = (uint8_t)(req->wValue);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  rx_tx: data transfer direction:
      \arg        USBD_TX
      \arg        USBD_RX
    \param[in]  ep_id: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
uint8_t  usbd_hid_data_handler (void *pudev, usb_dir_enum rx_tx, uint8_t ep_id)
{
    if ((USB_TX == rx_tx) && ((HID_IN_EP & 0x7FU) == ep_id)) {

        /* ensure that the FIFO is empty before a new transfer,
         * this condition could be caused by a new transfer 
         * before the end of the previous transfer
         */
        usbd_ep_fifo_flush(pudev, HID_IN_EP);

        prev_transfer_complete = 0x01;


        return USBD_OK;
    }
    else if ((USB_TX == rx_tx) && ((HID_FN_IN_EP & 0x7FU) == ep_id))
    {

        /* ensure that the FIFO is empty before a new transfer,
         * this condition could be caused by a new transfer
         * before the end of the previous transfer
         */
        usbd_ep_fifo_flush(pudev, HID_FN_IN_EP);

        prev_transfer_complete = 0x01;


        return USBD_OK;
    }
		else if ((USB_RX == rx_tx) && ((0x00 & 0x7fU) == ep_id))
		{
			//	handle led
			led_handler(usbd_hid_report_buffer[0]);
			return USBD_OK;
		}

    return USBD_FAIL;
}

/*!
    \brief      send keyboard report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to HID report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t usbd_hid_report_send(usb_core_handle_struct *pudev, uint8_t *report, uint16_t len, uint8_t ep_addr)
{
    /* check if USB is configured */
    prev_transfer_complete = 0U;

    usbd_ep_tx(pudev, ep_addr, report, len);

    return USBD_OK;
}
