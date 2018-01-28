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

#include "bluetooth-device.h"
#include "device.h"
#include "ldm-private.h"
#include "util.h"

struct _LdmBluetoothDeviceClass {
        LdmDeviceClass parent_class;
};

/**
 * SECTION:bluetooth-device
 * @Short_description: Bluetooth Device abstraction
 * @see_also: #LdmDevice
 * @Title: LdmBluetoothDevice
 *
 * An LdmBluetoothDevice is a specialised implementation of the #LdmDevice which
 * is aware of Bluetooth specific data. This class is never directly
 * created by the user, but is instead returned by the #LdmManager.
 *
 * Users can test if a device is a Bluetooth device without having to cast, by
 * simply checking the #LdmDevice:device-type:
 *
 * |[<!-- language="C" -->
 *      if (ldm_device_has_type(device, LDM_DEVICE_TYPE_BLUETOOTH)) {
 *              g_message("Found Bluetooth device");
 *      }
 *
 *      // Alternatively..
 *      if (LDM_IS_BLUETOOTH_DEVICE(device)) {
 *              g_message("Found Bluetooth device through casting");
 *      }
 * ]|
 */

/*
 * LdmBluetoothDevice
 *
 * An LdmBluetoothDevice is a specialised implementation of the #LdmDevice which
 * is aware of DMI interfaces
 */
struct _LdmBluetoothDevice {
        LdmDevice parent;
};

G_DEFINE_TYPE(LdmBluetoothDevice, ldm_bluetooth_device, LDM_TYPE_DEVICE)

/**
 * ldm_bluetooth_device_dispose:
 *
 * Clean up a LdmBluetoothDevice instance
 */
static void ldm_bluetooth_device_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_bluetooth_device_parent_class)->dispose(obj);
}

/**
 * ldm_bluetooth_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_bluetooth_device_class_init(LdmBluetoothDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_bluetooth_device_dispose;
}

/**
 * ldm_bluetooth_device_init:
 *
 * Handle construction of the LdmBluetoothDevice
 */
static void ldm_bluetooth_device_init(LdmBluetoothDevice *self)
{
        LdmDevice *ldm = LDM_DEVICE(self);
        ldm->os.devtype |= LDM_DEVICE_TYPE_BLUETOOTH;
        ldm->os.attributes |= LDM_DEVICE_ATTRIBUTE_INTERFACE; /* Allow USB to know about us */
}

/**
 * ldm_bluetooth_device_init_private:
 *
 * Add Bluetooth specific data, such as host controller state.
 */
void ldm_bluetooth_device_init_private(LdmDevice *self, udev_device *device)
{
        const char *devtype = NULL;

        /* Figure out if we're a host controller */
        devtype = udev_device_get_devtype(device);
        if (devtype && g_str_equal(devtype, "host")) {
                self->os.attributes |= LDM_DEVICE_ATTRIBUTE_HOST;
        }
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
