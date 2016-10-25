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

#include <stdio.h>
#include <stdlib.h>

#include "ldm.h"
#include "util.h"

int main(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        autofree(LdmManager) *manager = NULL;

        manager = ldm_manager_new();
        if (!ldm_manager_scan(manager)) {
                fputs("Failed to scan devices\n", stderr);
        }

        fputs("not yet implemented\n", stderr);

        return EXIT_SUCCESS;
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
