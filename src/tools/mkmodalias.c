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

#include <errno.h>
#include <glib.h>
#include <libkmod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_usage(const char *progname)
{
        fprintf(stderr, "%s usage:\n", progname);
        fprintf(stderr, "[packagename] [.ko files]\n");
}

static void examine_module(const char *package_name, struct kmod_module *module)
{
        const char *kname = NULL;
        struct kmod_list *list = NULL;
        struct kmod_list *iter = NULL;

        kname = kmod_module_get_name(module);

        /* Attempt probe */
        if (kmod_module_get_info(module, &list) < 0) {
                fprintf(stderr, "Couldn't probe module\n");
                return;
        };

        /* Walk all the aliases now */
        kmod_list_foreach(iter, list)
        {
                const char *key = kmod_module_info_get_key(iter);
                if (!key || !g_str_equal(key, "alias")) {
                        continue;
                }
                const char *value = kmod_module_info_get_value(iter);
                fprintf(stdout, "alias %s %s %s\n", value, kname, package_name);
        };

        /* Cleanup */
        kmod_module_info_free_list(list);
}

int main(int argc, char **argv)
{
        const char *package_name = NULL;
        const char *prog_name = argv[0];
        ++argv;
        --argc;
        struct kmod_ctx *ctx = NULL;

        if (argc < 2) {
                print_usage(prog_name);
                return EXIT_FAILURE;
        }

        package_name = argv[0];
        ++argv;
        --argc;

        /* Sanity - do they all exist */
        for (int i = 0; i < argc; i++) {
                if (access(argv[i], F_OK) != 0) {
                        fprintf(stderr, "Kernel module does not exist: %s\n", argv[i]);
                        return EXIT_FAILURE;
                }
                if (!g_str_has_suffix(argv[i], ".ko")) {
                        fprintf(stderr,
                                "File does not appear to be a kernel module: %s\n",
                                argv[i]);
                        return EXIT_FAILURE;
                }
        }

        ctx = kmod_new(NULL, NULL);
        if (!ctx) {
                fprintf(stderr, "Cannot init kmod..\n");
                return EXIT_FAILURE;
        }

        /* Walk all modules */
        for (int i = 0; i < argc; i++) {
                const char *kpath = argv[i];
                struct kmod_module *module = NULL;
                int ret = kmod_module_new_from_path(ctx, kpath, &module);
                if (ret != 0) {
                        fprintf(stderr, "Couldn't open module: %s %s\n", kpath, strerror(errno));
                        goto cleanup;
                }
                examine_module(package_name, module);
                kmod_module_unref(module);
        }

cleanup:
        kmod_unref(ctx);

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
