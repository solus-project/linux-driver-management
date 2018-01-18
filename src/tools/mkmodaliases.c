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

#define _GNU_SOURCE

#include "config.h"

#include <errno.h>
#include <glib.h>
#include <libkmod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_usage(const char *progname)
{
        fprintf(stderr, "%s usage: package-name [.ko files]\n", progname);
        fprintf(stderr, "Run '%s --help' for further information\n", progname);
}

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
/**
 * CLI options that we know about
 */

static gboolean opt_version = FALSE;
static gchar *opt_filename = NULL;
static gchar **opt_strings = NULL;

static GOptionEntry cli_entries[] = {
        { "version", 'v', 0, G_OPTION_ARG_NONE, &opt_version, "Print version and exit", NULL },
        { "output",
          'o',
          0,
          G_OPTION_ARG_FILENAME,
          &opt_filename,
          "Redirect to the given file",
          NULL },
        { G_OPTION_REMAINING,
          0,
          0,
          G_OPTION_ARG_STRING_ARRAY,
          &opt_strings,
          "Options",
          "- package-name [.ko file] [.ko file]" },
        { 0 },
};

/**
 * Examine just one kmod module and emit the info to the output file.
 */
static gboolean examine_module(const gchar *package_name, FILE *fileh, struct kmod_module *module)
{
        const char *kname = NULL;
        struct kmod_list *list = NULL;
        struct kmod_list *iter = NULL;
        gboolean ret = FALSE;

        kname = kmod_module_get_name(module);

        /* Attempt probe */
        if (kmod_module_get_info(module, &list) < 0) {
                fprintf(stderr, "Couldn't probe module '%s'\n", kname);
                return FALSE;
        };

        /* Walk all the aliases now */
        kmod_list_foreach(iter, list)
        {
                const char *key = kmod_module_info_get_key(iter);
                if (!key || !g_str_equal(key, "alias")) {
                        continue;
                }
                const char *value = kmod_module_info_get_value(iter);
                if (fprintf(fileh, "alias %s %s %s\n", value, kname, package_name) < 0) {
                        goto cleanup;
                }
        };

        /* All good */
        ret = TRUE;

cleanup:

        kmod_module_info_free_list(list);
        return ret;
}

/**
 * Construct a modaliases file for the given package name and module paths.
 */
static int mkmodaliases(const char *package_name, gchar **paths, guint n_paths)
{
        FILE *output_file = NULL;
        struct kmod_ctx *ctx = NULL;
        int ret = EXIT_FAILURE;

        /* Default to stdout if no path is set */
        if (opt_filename) {
                output_file = fopen(opt_filename, "w");
        } else {
                output_file = stdout;
        }

        /* Make sure we have something to write to */
        if (!output_file) {
                fprintf(stderr, "Failed to open %s for writing: %s", opt_filename, strerror(errno));
                return EXIT_FAILURE;
        }

        /* Open kmod context with no host kernel knowledge */
        ctx = kmod_new(NULL, NULL);
        if (!ctx) {
                fprintf(stderr, "Cannot init kmod: %s\n", strerror(errno));
                goto cleanup;
        }

        /* Walk all modules and pass our fileh */
        for (guint i = 0; i < n_paths; i++) {
                const gchar *kpath = paths[i];
                struct kmod_module *module = NULL;
                gboolean success = FALSE;

                int ret = kmod_module_new_from_path(ctx, kpath, &module);
                if (ret != 0) {
                        fprintf(stderr, "Couldn't open module: %s %s\n", kpath, strerror(errno));
                        goto cleanup;
                }
                success = examine_module(package_name, output_file, module);
                kmod_module_unref(module);
                if (!success) {
                        goto cleanup;
                }
        }

        /* All good so far */
        ret = EXIT_SUCCESS;

cleanup:
        if (output_file != stdout) {
                fflush(output_file);
                fclose(output_file);

                if (ret != EXIT_SUCCESS && unlink(opt_filename) != 0) {
                        fprintf(stderr,
                                "Failed to unlink() erronous output file %s: %s\n",
                                opt_filename,
                                strerror(errno));
                }
        }

        if (ctx) {
                kmod_unref(ctx);
        }

        return ret;
}

int main(int argc, char **argv)
{
        g_autoptr(GError) error = NULL;
        g_autoptr(GOptionContext) opt_context = NULL;
        int ret = EXIT_FAILURE;
        guint n_strings = 0;
        const gchar *package_name = NULL;

        opt_context = g_option_context_new(NULL);
        /* TODO: Set gettext up */
        g_option_context_add_main_entries(opt_context, cli_entries, "linux-driver-management");

        if (!g_option_context_parse(opt_context, &argc, &argv, &error)) {
                fprintf(stderr, "Failed to parse arguments: %s\n", error->message);
                goto cleanup;
        }

        if (opt_version) {
                print_version();
                ret = EXIT_SUCCESS;
                goto cleanup;
        }

        n_strings = opt_strings ? g_strv_length(opt_strings) : 0;
        if (n_strings < 2) {
                print_usage(argv[0]);
                goto cleanup;
        }

        package_name = opt_strings[0];

        /* Make sure they all exist now */
        for (guint i = 1; i < n_strings; i++) {
                if (access(opt_strings[i], F_OK) != 0) {
                        fprintf(stderr, "Kernel module does not exist: %s\n", opt_strings[i]);
                        goto cleanup;
                }
                if (!g_str_has_suffix(opt_strings[i], ".ko")) {
                        fprintf(stderr,
                                "File does not appear to be a kernel module: %s\n",
                                opt_strings[i]);
                        goto cleanup;
                }
        }

        /* All good, proceed. */
        ret = mkmodaliases(package_name, opt_strings + 1, n_strings - 1);

cleanup:
        if (opt_filename) {
                g_free(opt_filename);
        }
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
