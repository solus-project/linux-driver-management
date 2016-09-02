/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include <stdlib.h>

#include "atomics.h"
#include "ldm.h"

struct LdmManager {
        ldm_atomic_t atom;
};

static inline void ldm_manager_destroy(LdmManager *manager)
{
        if (!manager) {
                return;
        }
        free(manager);
}

__ldm_public__ LdmManager *ldm_manager_new(void)
{
        LdmManager *ret = NULL;

        ret = calloc(1, sizeof(LdmManager));
        if (!ret) {
                return NULL;
        }
        return ldm_atomic_init((ldm_atomic_t *)ret, (ldm_atomic_free)ldm_manager_destroy);
}

__ldm_public__ void ldm_manager_free(LdmManager *manager)
{
        ldm_atomic_unref(manager);
}
