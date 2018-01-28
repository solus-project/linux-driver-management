/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <libusb.h>
#include <stdlib.h>

#include "device.h"
#include "ldm-private.h"
#include "usb-device.h"
#include "util.h"

struct _LdmUSBDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:usb-device
 * @Short_description: USB Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmUSBDevice
 *
 * An LdmUSBDevice is a specialised implementation of the #LdmDevice which
 * is aware of UB interface capabilities. This class is never directly
 * created by the user, but is instead returned by the #LdmManager.
 *
 * This class extends the base #LdmDevice to add USB specific data.
 *
 * Users can test if a device is a USB device without having to cast, by
 * simply checking the #LdmDevice:device-type:
 *
 * |[<!-- language="C" -->
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_USB)) {
 *              g_message("Found PCI device");
 *      }
 *
 *      // Alternatively..
 *      if (LDM_IS_USB_DEVICE(device)) {
 *              g_message("Found PCI device through casting");
 *      }
 * ]|
 *
 * Internally USB devices are complex, having a top-level device visible
 * to the operating system and specialised interfaces (`bInterfaceClass`) exposed
 * via ports. #LdmUSBDevice abstracts that complexity away, and is instead composed
 * of multiple devices.
 *
 * Conceptually, a #LdmUSBDevice has each interface as a separate child within
 * the toplevel instance. Most end users will be able to safely ignore this difference,
 * as #LdmUSBDevice will modify the #LdmDevice:device-type to be the merged type
 * of the toplevel device, and all interfaces.
 *
 * As a result, any capabilities exposed by interfaces are available in the toplevel
 * object returned by the #LdmManager instance. This can be used to search for
 * specific types of USB devices:
 *
 * |[<!-- language="c" -->
 *      gint search_mask = LDM_DEVICE_TYPE_USB | LDM_DEVICE_TYPE_PRINTER;
 *      g_autoptr(GPtrArray) usb_printers = ldm_manager_get_devices(search_mask);
 *      g_message("Got %u printers", usb_printers->len);
 *      LdmDevice *device = usb_printers->pdata[0];
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_IMAGE)) {
 *              g_message("Printer supports PTP!");
 *      }
 * ]|
 */

/*
 * LdmUSBDevice
 *
 * An LdmUSBDevice is a specialised implementation of the #LdmDevice which
 * is aware of USB interfaces
 */
struct _LdmUSBDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmUSBDevice, ldm_usb_device, LDM_TYPE_DEVICE)

/**
 * ldm_usb_device_dispose:
 *
 * Clean up a LdmUSBDevice instance
 */
static void ldm_usb_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_usb_device_parent_class)->dispose(obj);
}

/**
 * ldm_usb_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_usb_device_class_init(LdmUSBDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_usb_device_dispose;
}

/**
 * ldm_usb_device_init:
 *
 * Handle construction of the LdmUSBDevice
 */
static void ldm_usb_device_init(LdmUSBDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_USB;
}

/**
 * ldm_usb_device_assign_class:
 * @bDeviceClass: The USB class for the device/interface
 *
 * Update our #LdmDevice device class based on the device/interface
 * class as mapped through libusb defines.
 */
static void ldm_usb_device_assign_class(LdmDevice *self, int bDeviceClass)
{
        switch (bDeviceClass) {
        case LIBUSB_CLASS_AUDIO:
                self->os.devtype |= LDM_DEVICE_TYPE_AUDIO;
                break;
        case LIBUSB_CLASS_HID:
                self->os.devtype |= LDM_DEVICE_TYPE_HID;
                break;
        case LIBUSB_CLASS_IMAGE:
                self->os.devtype |= LDM_DEVICE_TYPE_IMAGE;
                break;
        case LIBUSB_CLASS_PRINTER:
                self->os.devtype |= LDM_DEVICE_TYPE_PRINTER;
                break;
        case LIBUSB_CLASS_MASS_STORAGE:
                self->os.devtype |= LDM_DEVICE_TYPE_STORAGE;
                break;
        case LIBUSB_CLASS_VIDEO:
                self->os.devtype |= LDM_DEVICE_TYPE_VIDEO;
                break;
        case LIBUSB_CLASS_WIRELESS:
                self->os.devtype |= LDM_DEVICE_TYPE_WIRELESS;
                break;
        default:
                break;
        }
}

static void ldm_usb_device_assign_pvid(LdmDevice *self, udev_device *device)
{
        const char *sysattr = NULL;

        /* Grab the idVendor (hex) */
        sysattr = udev_device_get_sysattr_value(device, "idVendor");
        if (!sysattr) {
                goto product;
        }
        self->id.vendor_id = (gint)(strtoll(sysattr, NULL, 16));
        sysattr = NULL;

product:

        /* Grab the idProduct (hex) */
        sysattr = udev_device_get_sysattr_value(device, "idProduct");
        if (!sysattr) {
                return;
        }
        self->id.product_id = (gint)(strtoll(sysattr, NULL, 16));
}

/**
 * ldm_usb_device_init_private:
 * @device: The udev device that we're being created from
 *
 * Handle USB specific initialisation
 */
void ldm_usb_device_init_private(LdmDevice *self, udev_device *device)
{
        const gchar *devtype = NULL;
        const gchar *sysattr = NULL;
        int iface_class = 0;

        /* Is this a USB interface? If so, we're gonna need a parent. */
        devtype = udev_device_get_devtype(device);
        if (devtype && g_str_equal(devtype, "usb_interface")) {
                self->os.attributes |= LDM_DEVICE_ATTRIBUTE_INTERFACE;
                sysattr = udev_device_get_sysattr_value(device, "bInterfaceClass");
        } else {
                sysattr = udev_device_get_sysattr_value(device, "bDeviceClass");
        }

        ldm_usb_device_assign_pvid(self, device);

        if (!sysattr) {
                return;
        }

        iface_class = (int)strtoll(sysattr, NULL, 10);
        ldm_usb_device_assign_class(self, iface_class);
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
