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

#include "config.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static gboolean opt_version = FALSE;

static GOptionEntry cli_entries[] = {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, "Print version and exit", NULL },
        { 0 },
};

static void print_version(void)
{
        fputs(PACKAGE_NAME " version " PACKAGE_VERSION "\n\n", stdout);
        fputs("Copyright © 2017-2018 Solus Project\n\n", stdout);
        fputs(PACKAGE_NAME
              " "
              "is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU Lesser General Public License as published by\n\
the Free Software Foundation; either version 2.1 of the License, or\n\
(at your option) any later version.\n",
              stdout);
}

int main(int argc, char **argv)
{
        g_autoptr(GError) error = NULL;
        g_autoptr(GOptionContext) opt_context = NULL;

        opt_context = g_option_context_new(NULL);
        g_option_context_add_main_entries(opt_context, cli_entries, "linux-driver-management");

        if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
                fprintf(stderr, "Failed to parse arguments: %s\n", error->message);
                return EXIT_FAILURE;
        }

        if (opt_version) {
                print_version();
        }

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
