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

#include "device.h"
#include "dmi-device.h"
#include "ldm-private.h"
#include "util.h"

struct _LdmDMIDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:dmi-device
 * @Short_description: DMI Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmDMIDevice
 *
 * An LdmDMIDevice is a specialised implementation of the #LdmDevice which
 * is aware of DMI specific data. This class is never directly
 * created by the user, but is instead returned by the #LdmManager.
 *
 * This class extends the base #LdmDevice to add DMI specific data.
 * This is almost exclusively used to provide platform identification and
 * allow matching of specific hardware products (i.e. laptops) via their
 * modalias.
 *
 * Users can test if a device is a DMI device without having to cast, by
 * simply checking the #LdmDevice:device-type:
 *
 * |[<!-- language="C" -->
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_PLATFORM)) {
 *              g_message("Found PCI device");
 *      }
 *
 *      // Alternatively..
 *      if (LDM_IS_DMI_DEVICE(device)) {
 *              g_message("Found PCI device through casting");
 *      }
 * ]|
 */

/*
 * LdmDMIDevice
 *
 * An LdmDMIDevice is a specialised implementation of the #LdmDevice which
 * is aware of DMI interfaces
 */
struct _LdmDMIDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmDMIDevice, ldm_dmi_device, LDM_TYPE_DEVICE)

/**
 * ldm_dmi_device_dispose:
 *
 * Clean up a LdmDMIDevice instance
 */
static void ldm_dmi_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_dmi_device_parent_class)->dispose(obj);
}

/**
 * ldm_dmi_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_dmi_device_class_init(LdmDMIDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_dmi_device_dispose;
}

/**
 * ldm_dmi_device_init:
 *
 * Handle construction of the LdmDMIDevice
 */
static void ldm_dmi_device_init(LdmDMIDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_PLATFORM;
}

/**
 * ldm_dmi_device_init_private:
 * @device: The udev device that we're being created from
 *
 * Handle DMI specific initialisation
 */
void ldm_dmi_device_init_private(LdmDevice *self, __ldm_unused__ udev_device *device)
{
        const char *sysattr = NULL;

        sysattr = udev_device_get_sysattr_value(device, "board_vendor");
        self->id.vendor = sysattr ? g_strdup(sysattr) : g_strdup("Unknown Vendor");
        sysattr = NULL;

        sysattr = udev_device_get_sysattr_value(device, "board_name");
        self->id.name = sysattr ? g_strdup(sysattr) : g_strdup("Platform device");
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
