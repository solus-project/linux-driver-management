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
};

G_DEFINE_TYPE(LdmDevice, ldm_device, G_TYPE_OBJECT)

/**
 * ldm_device_dispose:
 *
 * Clean up a LdmDevice instance
 */
static void ldm_device_dispose(GObject *obj)
{
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
