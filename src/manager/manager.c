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

#include "ldm.h"

struct LdmManager {
        int __reserved1;
};

LdmManager *ldm_manager_new(void)
{
        LdmManager *ret = NULL;

        ret = calloc(1, sizeof(LdmManager));
        return ret;
}

void ldm_manager_free(LdmManager *manager)
{
        if (!manager) {
                return;
        }
        free(manager);
}
