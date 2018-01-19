/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include "../lib/util.h"
#include "cli.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static void print_usage(void)
{
        fputs("configure takes exactly one argument: gpu\n", stderr);
}

static int ldm_cli_configure_gpu(void)
{
        fputs("Not yet implemented\n", stderr);
        return EXIT_FAILURE;
}

int ldm_cli_configure(int argc, char **argv)
{
        if (argc != 2) {
                print_usage();
                return EXIT_FAILURE;
        }

        if (g_str_equal(argv[1], "gpu")) {
                return ldm_cli_configure_gpu();
        }

        print_usage();
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
