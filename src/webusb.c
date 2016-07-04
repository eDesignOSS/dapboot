/*
 * Copyright (c) 2016, Devan Lai
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <string.h>

#include <libopencm3/usb/usbd.h>
#include "webusb.h"

#include "usb_conf.h"

#include "config.h"

const struct webusb_platform_descriptor webusb_platform = {
    .bLength = WEBUSB_PLATFORM_DESCRIPTOR_SIZE,
    .bDescriptorType = USB_DT_DEVICE_CAPABILITY,
    .bDevCapabilityType = USB_DC_PLATFORM,
    .bReserved = 0,
    .platformCapabilityUUID = WEBUSB_UUID,
    .bcdVersion = 0x0100,
    .bVendorCode = WEBUSB_VENDOR_CODE,
    .iLandingPage = 1
};

static const struct webusb_simple_origins webusb_origins = {
    .allowed_origins_header = {
        .bLength = WEBUSB_ALLOWED_ORIGINS_HEADER_SIZE,
        .bDescriptorType = WEBUSB_DT_DESCRIPTOR_SET_HEADER,
        .wTotalLength = (WEBUSB_ALLOWED_ORIGINS_HEADER_SIZE
                         + WEBUSB_CONFIGURATION_SUBSET_HEADER_SIZE
                         + WEBUSB_FUNCTION_SUBSET_HEADER_SIZE + 1),
        .bNumConfigurations = 1,
    },
    .configuration_subset_header = {
        .bLength = WEBUSB_CONFIGURATION_SUBSET_HEADER_SIZE,
        .bDescriptorType = WEBUSB_DT_CONFIGURATION_SUBSET_HEADER,
        .bConfigurationValue = 1,
        .bNumFunctions = 1,
    },
    .function_subset_header = {
        .bLength = (WEBUSB_FUNCTION_SUBSET_HEADER_SIZE + 1),
        .bDescriptorType = WEBUSB_DT_FUNCTION_SUBSET_HEADER,
        .bFirstInterfaceNumber = INTF_DFU,
    },
    .iOrigin = { 1 },
};

static const struct webusb_url_descriptor webusb_origin_url = {
    .bLength = (WEBUSB_DT_URL_DESCRIPTOR_SIZE + sizeof(LANDING_PAGE_URL)-1),
    .bDescriptorType = WEBUSB_DT_URL,
    .bScheme = WEBUSB_URL_SCHEME_HTTPS,
    .URL = LANDING_PAGE_URL,
};

static int webusb_control_vendor_request(usbd_device *usbd_dev,
                                     struct usb_setup_data *req,
                                     uint8_t **buf, uint16_t *len,
                                     usbd_control_complete_callback* complete) {
    (void)complete;
    (void)usbd_dev;

    if (req->bRequest != WEBUSB_VENDOR_CODE) {
        return USBD_REQ_NEXT_CALLBACK;
    }

    int status = USBD_REQ_NOTSUPP;
    switch (req->wIndex) {
        case WEBUSB_REQ_GET_ALLOWED_ORIGINS: {
            if (*len > webusb_origins.allowed_origins_header.wTotalLength) {
                *len = webusb_origins.allowed_origins_header.wTotalLength;
            }
            
            *buf = (uint8_t*)&webusb_origins;
            status = USBD_REQ_HANDLED;
            break;
        }
        case WEBUSB_REQ_GET_URL: {
            uint16_t index = req->wValue;
            if (index == 1) {
                *len = webusb_origin_url.bLength;
                *buf = (uint8_t*)&webusb_origin_url;
                status = USBD_REQ_HANDLED;
            } else {
                // TODO: stall instead?
                status = USBD_REQ_NOTSUPP;
            }
            break;
        }
        default: {
            status = USBD_REQ_NOTSUPP;
            break;
        }
    }

    return status;
}

static void webusb_set_config(usbd_device* usbd_dev, uint16_t wValue) {
    (void)wValue;
    usbd_register_control_callback(
        usbd_dev,
        USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_DEVICE,
        USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
        webusb_control_vendor_request);
}

void webusb_setup(usbd_device* usbd_dev) {
    (void)usbd_dev;
    usbd_register_set_config_callback(usbd_dev, webusb_set_config);
}
