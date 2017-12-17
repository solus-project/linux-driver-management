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

#include <stdbool.h>
#include <stdlib.h>

#include "ldm.h"
#include "monitor.h"
#include "util.h"

struct LdmMonitor {
        LdmManager *manager;
};

LdmMonitor *ldm_monitor_new(void)
{
        LdmMonitor *ret = NULL;

        ret = g_new0(LdmMonitor, 1);
        if (!ret) {
                return NULL;
        }
        ret->manager = ldm_manager_new(LDM_MANAGER_FLAGS_NONE);

        return ret;
}

void ldm_monitor_free(LdmMonitor *self)
{
        g_clear_object(&self->manager);
        g_free(self);
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
