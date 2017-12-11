/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2017 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <libusb.h>
#include <stdlib.h>
#include <string.h>

#include "ldm-private.h"
#include "usb.h"

LdmDevice *ldm_scan_usb_devices(__ldm_unused__ unsigned int classmask)
{
        libusb_init(NULL);
        ssize_t n_usbs = 0;
        libusb_device **usb_devices = NULL;
        struct libusb_device_descriptor usb_desc = { 0 };
        LdmDevice *ret = NULL;
        bool errored = true;

        n_usbs = libusb_get_device_list(NULL, &usb_devices);
        if (n_usbs < 0) {
                goto error;
        }

        fprintf(stderr, "debug(): %ld USB devices\n", n_usbs);

        for (ssize_t i = 0; i < n_usbs; i++) {
                libusb_device *device = usb_devices[i];
                int ret = 0;

                ret = libusb_get_device_descriptor(device, &usb_desc);
                if (ret < 0) {
                        goto error;
                }

                fprintf(stderr, "Got device: %d\n", usb_desc.bDeviceClass);
        }

        errored = false;

error:
        if (errored) {
                fprintf(stderr, "Failed to retrieve USB devices: %s", libusb_strerror(n_usbs));
        }

        if (usb_devices) {
                libusb_free_device_list(usb_devices, (int)n_usbs);
        }

        libusb_exit(NULL);
        return ret;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
