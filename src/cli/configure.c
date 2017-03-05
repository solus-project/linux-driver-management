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

#include <stdio.h>
#include <stdlib.h>

#include "cli.h"
#include "config.h"
#include "util.h"

/**
 * Updated as and when we add more driver types
 */
static const char *known_driver_types = "gpu";

/**
 * Configure the system GPU
 */
int ldm_cli_configure_gpu(void)
{
        fputs("Not yet implemented\n", stderr);
        return EXIT_FAILURE;
}

int ldm_cli_configure(int argc, char **argv)
{
        if (argc != 1) {
                fputs("Incorrect usage. Expected keyword\n", stderr);
                goto emit_types;
        }
        if (streq(argv[0], "gpu")) {
                return ldm_cli_configure_gpu();
        }
        fprintf(stderr, "Unknown driver type: '%s'\n", argv[0]);
emit_types:
        fprintf(stderr, "Known types: %s\n", known_driver_types);
        return EXIT_FAILURE;
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
