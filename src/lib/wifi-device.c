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

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ldm-private.h"
#include "util.h"
#include "wifi-device.h"

struct _LdmWifiDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:wifi-device
 * @Short_description: WiFI Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmWifiDevice
 *
 * An LdmWifiDevice is a specialised implementation of the #LdmDevice which
 * is aware of WiFi capabilities.
 */
struct _LdmWifiDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmWifiDevice, ldm_wifi_device, LDM_TYPE_DEVICE)

/**
 * ldm_wifi_device_dispose:
 *
 * Clean up a LdmWifiDevice instance
 */
static void ldm_wifi_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_wifi_device_parent_class)->dispose(obj);
}

/**
 * ldm_wifi_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_wifi_device_class_init(LdmWifiDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_wifi_device_dispose;
}

/**
 * ldm_wifi_device_init:
 *
 * Handle construction of the LdmWifiDevice
 */
static void ldm_wifi_device_init(LdmWifiDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_WIRELESS;
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
