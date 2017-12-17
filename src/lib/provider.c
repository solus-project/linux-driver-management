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

#include "provider.h"
#include "util.h"

/**
 * SECTION:provider
 * @Short_description: Hardware enabling information
 * @see_also: #LdmDevice, #LdmManager
 * @Title: LdmProvider
 *
 * An LdmProvider is the result type when searching for hardware providers
 * using the #LdmManager plugins.
 */

struct _LdmProviderClass {
        GObjectClass parent_class;
};

struct _LdmProvider {
        GObject parent;
};

G_DEFINE_TYPE(LdmProvider, ldm_provider, G_TYPE_OBJECT)

/**
 * ldm_provider_dispose:
 *
 * Clean up a LdmProvider instance
 */
static void ldm_provider_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_provider_parent_class)->dispose(obj);
}

/**
 * ldm_provider_class_init:
 *
 * Handle class initialisation
 */
static void ldm_provider_class_init(LdmProviderClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_provider_dispose;
}

/**
 * ldm_provider_init:
 *
 * Handle construction of the LdmProvider
 */
static void ldm_provider_init(__ldm_unused__ LdmProvider *self)
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
