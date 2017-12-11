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

#include "device.h"
#include "ldm-private.h"
#include "usb.h"

/**
 * Convert the USB class to an Ldm known class
 *
 * This only handles well known types, in future we'll also need to support
 * vendor/application specific descriptors for external GPUs, etc.
 */
static unsigned int ldm_usb_to_device_class(int bDeviceClass)
{
        switch (bDeviceClass) {
        case LIBUSB_CLASS_AUDIO:
                return LDM_CLASS_AUDIO;
        case LIBUSB_CLASS_HID:
                return LDM_CLASS_HID;
        case LIBUSB_CLASS_PRINTER:
                return LDM_CLASS_PRINTER;
        case LIBUSB_CLASS_VIDEO:
                return LDM_CLASS_VIDEO;
        case LIBUSB_CLASS_WIRELESS:
                return LDM_CLASS_WIRELESS;
        default:
                return 0;
        }
}

static unsigned int accumulate_device_classes(libusb_device *device,
                                              struct libusb_device_descriptor *desc)
{
        unsigned int accum = 0;
        struct libusb_config_descriptor *config = NULL;

        if (desc->bDeviceClass != LIBUSB_CLASS_PER_INTERFACE) {
                return ldm_usb_to_device_class(desc->bDeviceClass);
        }

        libusb_get_config_descriptor(device, 0, &config);

        for (size_t i = 0; i < desc->bNumConfigurations; i++) {
                const struct libusb_interface *iface = NULL;

                iface = &config->interface[i];
                for (ssize_t j = 0; j < iface->num_altsetting; j++) {
                        const struct libusb_interface_descriptor *idesc = NULL;

                        idesc = &iface->altsetting[j];
                        accum |= ldm_usb_to_device_class(idesc->bInterfaceClass);
                }
        }

        if (config) {
                libusb_free_config_descriptor(config);
        }

        return accum;
}

static char *ldm_usb_device_sysfs_path(libusb_device *device)
{
        return string_printf("/sys/bus/usb/devices/%d-%d",
                             libusb_get_bus_number(device),
                             libusb_get_device_address(device));
}

static LdmDevice *ldm_usb_device_new(libusb_device *device)
{
        LdmDevice *ret = NULL;
        LdmUSBDevice *usb = NULL;

        usb = calloc(1, sizeof(LdmUSBDevice));
        if (!usb) {
                goto oom_fail;
        }
        ret = (LdmDevice *)usb;

        /* Finish off the structure */
        ret->type = LDM_DEVICE_USB;
        ret->sysfs_address = ldm_usb_device_sysfs_path(device);
        ret->driver = ldm_device_driver(ret);

        return ret;
oom_fail:
        fputs("Out of memory", stderr);
        abort();
        return NULL;
}

LdmDevice *ldm_scan_usb_devices(unsigned int classmask)
{
        libusb_init(NULL);
        ssize_t n_usbs = 0;
        libusb_device **usb_devices = NULL;
        struct libusb_device_descriptor usb_desc = { 0 };
        LdmDevice *root = NULL;
        bool errored = true;

        n_usbs = libusb_get_device_list(NULL, &usb_devices);
        if (n_usbs < 0) {
                goto error;
        }

        fprintf(stderr, "debug(): %ld USB devices\n", n_usbs);

        for (ssize_t i = 0; i < n_usbs; i++) {
                libusb_device *device = usb_devices[i];
                int ret = 0;
                LdmDeviceClass class = 0;
                LdmDevice *dev = NULL;

                ret = libusb_get_device_descriptor(device, &usb_desc);
                if (ret < 0) {
                        goto error;
                }

                class = accumulate_device_classes(device, &usb_desc);

                /* Skip unrequested types */
                if ((class & classmask) != classmask) {
                        continue;
                }

                dev = ldm_usb_device_new(device);
                if (!dev) {
                        goto error;
                }

                dev->next = root;
                root = dev;

                printf("Got a device\n");
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
        return root;
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
