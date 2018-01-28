/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#include "cli.h"
#include "config.h"
#include "util.h"

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>

static gboolean opt_version = FALSE;
static gchar **opt_strings = NULL;

static GOptionEntry cli_entries[] = {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, "Print version and exit", NULL },
        { G_OPTION_REMAINING,
          0,
          0,
          G_OPTION_ARG_STRING_ARRAY,
          &opt_strings,
          "Options",
          "- [subcommand]" },
        { 0 },
};

static void print_usage(const char *progname)
{
        fprintf(stderr, "%s usage: [status]\n", progname);
        fprintf(stderr, "Run '%s --help' for further information\n", progname);
}

#ifndef WITH_GLX_CONFIGURATION
static int ldm_cli_configure(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        fprintf(stderr, "GLX configuration has been disabled in this build\n");
        return EXIT_FAILURE;
}
#endif

int main(int argc, char **argv)
{
        g_autoptr(GError) error = NULL;
        g_autoptr(GOptionContext) opt_context = NULL;
        guint n_strings = 0;
        int ret = EXIT_FAILURE;
        ldm_cli_command command = NULL;

        opt_context = g_option_context_new(NULL);
        g_option_context_add_main_entries(opt_context, cli_entries, "linux-driver-management");
        g_option_context_set_summary(opt_context,
                                     "Interface with the linux-driver-management library");
        g_option_context_set_description(opt_context,
                                         "This tool accepts a number of subcommands:\n\
\n\
        configure   - Attempt configuration of a subsystem\n\
        status      - Emit the status for known, detected devices\n\
        version     - Print the version and quit\n\
");

        if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
                fprintf(stderr, "Failed to parse arguments: %s\n", error->message);
                return EXIT_FAILURE;
        }

        n_strings = opt_strings ? g_strv_length(opt_strings) : 0;

        /* Early handle version request */
        if (opt_version) {
                ret = ldm_cli_version((int)n_strings, opt_strings);
                goto cleanup;
        }

        /* We expect at least one argument */
        if (n_strings < 1) {
                print_usage(argv[0]);
                goto cleanup;
        }

        if (g_str_equal(opt_strings[0], "status")) {
                command = &ldm_cli_status;
        } else if (g_str_equal(opt_strings[0], "configure")) {
                command = &ldm_cli_configure;
        } else if (g_str_equal(opt_strings[0], "version")) {
                command = &ldm_cli_version;
        } else {
                fprintf(stderr, "Unknown command '%s'\n", opt_strings[0]);
                ret = EXIT_FAILURE;
                goto cleanup;
        }

        /* Execute helper */
        ret = command((int)n_strings, opt_strings);

cleanup:
        if (opt_strings && *opt_strings) {
                g_strfreev(opt_strings);
        }

        return ret;
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
