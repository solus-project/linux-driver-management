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

#include "modalias-driver.h"
#include "util.h"

struct _LdmModaliasDriverClass {
        LdmDriverClass parent_class;
};

/*
 * LdmModaliasDriver
 *
 * The LdmModaliasDriver extends the base #LdmDriver and adds modalias-based
 * hardware detection to it.
 */
struct _LdmModaliasDriver {
        LdmDriver parent;
};

G_DEFINE_TYPE(LdmModaliasDriver, ldm_modalias_driver, LDM_TYPE_DRIVER)

/**
 * ldm_modalias_driver_dispose:
 *
 * Clean up a LdmModaliasDriver instance
 */
static void ldm_modalias_driver_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_modalias_driver_parent_class)->dispose(obj);
}

/**
 * ldm_modalias_driver_class_init:
 *
 * Handle class initialisation
 */
static void ldm_modalias_driver_class_init(LdmModaliasDriverClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_modalias_driver_dispose;
}

/**
 * ldm_modalias_driver_init:
 *
 * Handle construction of the LdmModaliasDriver
 */
static void ldm_modalias_driver_init(__ldm_unused__ LdmModaliasDriver *self)
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
