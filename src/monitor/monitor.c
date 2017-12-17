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

#include "monitor.h"
#include "ldm.h"

struct _LdmDaemonClass {
        GObjectClass parent_class;
};

struct _LdmDaemon {
        GObject parent;

        LdmManager *manager;
};

G_DEFINE_TYPE(LdmDaemon, ldm_daemon, G_TYPE_OBJECT)

/**
 * ldm_daemon_dispose:
 *
 * Clean up a LdmDaemon instance
 */
static void ldm_daemon_dispose(GObject *obj)
{
        LdmDaemon *self = LDM_DAEMON(obj);

        g_clear_object(&self->manager);

        G_OBJECT_CLASS(ldm_daemon_parent_class)->dispose(obj);
}

/**
 * ldm_daemon_class_init:
 *
 * Handle class initialisation
 */
static void ldm_daemon_class_init(LdmDaemonClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_daemon_dispose;
}

/**
 * ldm_daemon_init:
 *
 * Handle construction of the LdmDaemon
 */
static void ldm_daemon_init(LdmDaemon *self)
{
        self->manager = ldm_manager_new(LDM_MANAGER_FLAGS_NONE);
}

/**
 * ldm_daemon_new:
 *
 * Returns: (transfer full): A new #LdmDaemon instance
 */
LdmDaemon *ldm_daemon_new()
{
        return g_object_new(LDM_TYPE_DAEMON, NULL);
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
