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

#include "util.h"

#include "device.h"

struct _LdmDeviceClass {
        GObjectClass parent_class;
};

/*
 * LdmDevice
 *
 * An LdmDevice is an opaque representation of an available device on
 * the system, and provides introspection opportunities to discover
 * capabilities, drivers, etc.
 */
struct _LdmDevice {
        GObject parent;

        /* OS Specifics */
        gchar *sysfs_path;
        gchar *modalias;

        /* Display */
        gchar *name;
        gchar *vendor;
};

G_DEFINE_TYPE(LdmDevice, ldm_device, G_TYPE_OBJECT)

/**
 * ldm_device_dispose:
 *
 * Clean up a LdmDevice instance
 */
static void ldm_device_dispose(GObject *obj)
{
        LdmDevice *self = LDM_DEVICE(obj);

        g_clear_pointer(&self->sysfs_path, g_free);
        g_clear_pointer(&self->modalias, g_free);
        g_clear_pointer(&self->name, g_free);
        g_clear_pointer(&self->vendor, g_free);

        G_OBJECT_CLASS(ldm_device_parent_class)->dispose(obj);
}

/**
 * ldm_device_class_init:
 *
 * Handle class initialisation
 */
static void ldm_device_class_init(LdmDeviceClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_device_dispose;
}

/**
 * ldm_device_init:
 *
 * Handle construction of the LdmDevice
 */
static void ldm_device_init(__ldm_unused__ LdmDevice *self)
{
}

/**
 * ldm_device_get_modalias:
 *
 * The modalias is unique to the device and is used in identifying potential
 * driver candidates.
 *
 * Returns: (transfer none): The modalias of the device
 */
const gchar *ldm_device_get_modalias(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->modalias;
}

/**
 * ldm_device_get_name:
 *
 * This function will return the name (model) of this device, suitable
 * for presentation to a user.
 *
 * Returns: (transfer none): The name of the device
 */
const gchar *ldm_device_get_name(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->name;
}

/**
 * ldm_device_get_path:
 *
 * This function will return the system-specific path for this device.
 * On Linux this is the sysfs path.
 *
 * Returns: (transfer none): The path of the device
 */
const gchar *ldm_device_get_path(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->sysfs_path;
}

/**
 * ldm_device_get_vendor:
 *
 * This function will return the vendor (manufacturer) of this device,
 * suitable for presentation to a user.
 *
 * Returns: (transfer none): The vendor of the device
 */
const gchar *ldm_device_get_vendor(LdmDevice *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->vendor;
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
