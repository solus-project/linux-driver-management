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

#include <stdlib.h>

#include "device.h"
#include "hid-device.h"
#include "ldm-private.h"
#include "util.h"

struct _LdmHIDDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:hid-device
 * @Short_description: HID Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmHIDDevice
 *
 * An LdmHIDDevice is a specialised implementation of the #LdmDevice which
 * is aware of HID interface capabilities. This class is never directly
 * created by the user, but is instead returned by the #LdmManager.
 *
 * This class extends the base #LdmDevice to add HID specific data.
 * Typically these devices are part of a USB device tree, and extend the
 * root USB device presented to the end user.
 *
 * Users can test if a device is a HID device without having to cast, by
 * simply checking the #LdmDevice:device-type:
 *
 * |[<!-- language="C" -->
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_HID)) {
 *              g_message("Found HID device");
 *      }
 *
 *      // Alternatively..
 *      if (LDM_IS_HID_DEVICE(device)) {
 *              g_message("Found HID device through casting");
 *      }
 * ]|
 *
 */

/*
 * LdmHIDDevice
 *
 * An LdmHIDDevice is a specialised implementation of the #LdmDevice which
 * is aware of HID interfaces
 */
struct _LdmHIDDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmHIDDevice, ldm_hid_device, LDM_TYPE_DEVICE)

/**
 * ldm_hid_device_dispose:
 *
 * Clean up a LdmHIDDevice instance
 */
static void ldm_hid_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_hid_device_parent_class)->dispose(obj);
}

/**
 * ldm_hid_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_hid_device_class_init(LdmHIDDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_hid_device_dispose;
}

/**
 * ldm_hid_device_init:
 *
 * Handle construction of the LdmHIDDevice
 */
static void ldm_hid_device_init(LdmHIDDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_HID;
        ldm->os.attributes |= LDM_DEVICE_ATTRIBUTE_INTERFACE; /* Allow USB to know about us */
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
