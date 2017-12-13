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

#include "modalias.h"
#include "util.h"

struct _LdmModaliasClass {
        GObjectClass parent_class;
};

/*
 * LdmModalias
 *
 * An LdmModalias is a mapping from an `fnmatch` style modalias match to
 * the required package name and kernel module.
 *
 * This is used in hardware detection to determine which package provides
 * the required kernel modules for any given hardware.
 */
struct _LdmModalias {
        GObject parent;
};

G_DEFINE_TYPE(LdmModalias, ldm_modalias, G_TYPE_OBJECT)

/**
 * ldm_modalias_dispose:
 *
 * Clean up a LdmModalias instance
 */
static void ldm_modalias_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_modalias_parent_class)->dispose(obj);
}

/**
 * ldm_modalias_class_init:
 *
 * Handle class initialisation
 */
static void ldm_modalias_class_init(LdmModaliasClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_modalias_dispose;
}

/**
 * ldm_modalias_init:
 *
 * Handle construction of the LdmModalias
 */
static void ldm_modalias_init(__ldm_unused__ LdmModalias *self)
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
