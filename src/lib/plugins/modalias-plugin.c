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
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "modalias-plugin.h"
#include "util.h"

struct _LdmModaliasPluginClass {
        LdmPluginClass parent_class;
};

static LdmProvider *ldm_modalias_plugin_get_provider(LdmPlugin *plugin, LdmDevice *device);

/**
 * SECTION:modalias-plugin
 * @Short_description: Modalias-based plugin implementation
 * @see_also: #LdmModalias, #LdmPlugin
 * @Title: LdmModaliasPlugin
 *
 * The LdmModaliasPlugin extends the base #LdmPlugin and adds modalias-based
 * hardware detection to it. This allows us to perform #LdmModalias based
 * matching against any given hardware.
 *
 * The most useful case for #LdmModaliasPlugin is to dynamically construct
 * it at runtime using `.modaliases` files. These files contain specially
 * formatted lines that can be parsed into a set of #LdmModalias rules,
 * all of which can later be used to detect hardware in bulk.
 *
 * The format of the file is very simple, space separated, with just 4
 * columns:
 *
 *      `TYPE   ALIAS   PLUGIN  PACKAGE`
 *
 * `TYPE` is reserved and is currently always set to `alias`.
 * `MODALIAS` is the `fnmatch` style string used to match against hardware.
 * `PLUGIN` is the kernel driver that supports any hardware matching the #LdmModlias:match field
 * `PACKAGE` is the name of the package or bundle providing the kernel driver
 *
 * Example:
 *
 *      `alias pci:v000014E4d*sv*sd*bc02sc80i* wl broadcom-sta`
 *
 * If a hardware device matching `pci:v000014E4d*sv*sd*bc02sc80i` is discovered,
 * it requires `wl.ko` to operate correctly (or to enhance it). The user can find
 * `wl.ko` in the `broadcom-sta` package.
 */
struct _LdmModaliasPlugin {
        LdmPlugin parent;

        /* Our known modalias implementations */
        GHashTable *modaliases;
};

G_DEFINE_TYPE(LdmModaliasPlugin, ldm_modalias_plugin, LDM_TYPE_PLUGIN)

/**
 * ldm_modalias_plugin_dispose:
 *
 * Clean up a LdmModaliasPlugin instance
 */
static void ldm_modalias_plugin_dispose(GObject *obj)
{
        LdmModaliasPlugin *self = LDM_MODALIAS_PLUGIN(obj);

        g_clear_pointer(&self->modaliases, g_hash_table_unref);

        G_OBJECT_CLASS(ldm_modalias_plugin_parent_class)->dispose(obj);
}

/**
 * ldm_modalias_plugin_class_init:
 *
 * Handle class initialisation
 */
static void ldm_modalias_plugin_class_init(LdmModaliasPluginClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);
        LdmPluginClass *plug_class = LDM_PLUGIN_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_modalias_plugin_dispose;

        /* plugin vtable hookup */
        plug_class->get_provider = ldm_modalias_plugin_get_provider;
}

/**
 * ldm_modalias_plugin_init:
 *
 * Handle construction of the LdmModaliasPlugin
 */
static void ldm_modalias_plugin_init(LdmModaliasPlugin *self)
{
        /* Map name to modalias */
        self->modaliases = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
}

/**
 * ldm_modalias_plugin_new:
 * @name: Name for this plugin instance
 *
 * Create a new LdmPlugin for modalias detection with the given name
 *
 * Returns: (transfer full): A newly initialised LdmModaliasPlugin
 */
LdmPlugin *ldm_modalias_plugin_new(const gchar *name)
{
        return g_object_new(LDM_TYPE_MODALIAS_PLUGIN, "name", name, "priority", 0, NULL);
}

/**
 * ldm_modalias_plugin_new_from_filename:
 * @filename: Path to a modaliases file
 *
 * Create a new LdmPlugin for modalias detection. The named file will be
 * opened and the resulting plugin will be seeded from that file.
 *
 * Returns: (transfer full): A newly initialised LdmModaliasPlugin
 */
LdmPlugin *ldm_modalias_plugin_new_from_filename(const gchar *filename)
{
        FILE *fp = NULL;
        char *bfr = NULL;
        size_t n = 0;
        ssize_t read = 0;
        LdmPlugin *ret = NULL;
        g_autofree gchar *path = NULL;

        g_return_val_if_fail(filename != NULL, NULL);
        if (access(filename, F_OK) != 0) {
                return NULL;
        }

        fp = fopen(filename, "r");
        if (!fp) {
                fprintf(stderr, "Failed to open %s: %s\n", filename, strerror(errno));
                return NULL;
        }

        /* Strip suffix if set */
        path = g_path_get_basename(filename);
        if (g_str_has_suffix(path, ".modaliases")) {
                path[strlen(path) - strlen(".modaliases")] = '\0';
        }

        ret = ldm_modalias_plugin_new(path);

        /* Walk the line. */
        while ((read = getline(&bfr, &n, fp)) > 0) {
                gchar *work = NULL;
                gchar **splits = NULL;
                g_autoptr(LdmModalias) alias = NULL;

                /* Strip the newline from it */
                if (bfr[read - 1] == '\n') {
                        bfr[read - 1] = '\0';
                        --read;
                }

                /* Empty lines are uninteresting. */
                if (read < 1) {
                        continue;
                }

                work = g_strstrip(bfr);

                /* Comment? */
                if (g_str_has_prefix(work, "#")) {
                        goto next_line;
                }

                splits = g_strsplit(work, " ", 4);
                if (!splits || g_strv_length(splits) != 4) {
                        goto next_line;
                }

                if (!g_str_equal(splits[0], "alias")) {
                        g_warning("unknown directive '%s'", splits[0]);
                        goto next_line;
                }

                alias = ldm_modalias_new(splits[1], splits[2], splits[3]);
                if (!alias) {
                        g_warning("invalid alias '%s'", work);
                        goto next_line;
                }

                /* Add modalias. */
                ldm_modalias_plugin_add_modalias(LDM_MODALIAS_PLUGIN(ret), alias);

        next_line:
                if (splits) {
                        g_strfreev(splits);
                }
                free(bfr);
                bfr = NULL;
        }

        if (bfr) {
                free(bfr);
        }

        fclose(fp);

        return ret;
}

/**
 * ldm_modalias_plugin_add_modalias:
 * @modalias: (transfer full): Modalias object to add to the table
 *
 * Add a new modalias object to the plugin table. This method will take a new
 * reference to the modalias.
 */
void ldm_modalias_plugin_add_modalias(LdmModaliasPlugin *self, LdmModalias *modalias)
{
        const gchar *id = NULL;

        g_return_if_fail(self != NULL);
        g_return_if_fail(modalias != NULL);

        id = ldm_modalias_get_match(modalias);
        g_assert(id != NULL);

        g_hash_table_replace(self->modaliases, g_strdup(id), g_object_ref_sink(modalias));
}

/**
 * ldm_modalias_plugin_is_installed:
 *
 * Janky helper to work out if a modalias plugin is installed.
 * This is only really useful to the monitor.
 */
static gboolean ldm_modalias_plugin_is_installed(LdmModalias *modalias)
{
        g_autofree gchar *sysfs_path = NULL;

        /* For now we only check kernel modules! */
        sysfs_path = g_strdup_printf("/sys/module/%s", ldm_modalias_get_driver(modalias));

        return g_file_test(sysfs_path, G_FILE_TEST_EXISTS);
}

/**
 * ldm_modalias_plugin_get_provider:
 * @device: Test input device
 *
 * Walk our modaliases and attempt to find a match from one of those lines. If
 * we match the device off against our table, return a new #LdmProvider to help
 * configure that device.
 *
 * Returns: (transfer full) (nullable): A new #LdmProvider for the device
 */
static LdmProvider *ldm_modalias_plugin_get_provider(LdmPlugin *plugin, LdmDevice *device)
{
        LdmModaliasPlugin *self = LDM_MODALIAS_PLUGIN(plugin);
        LdmProvider *ret = NULL;
        GHashTableIter iter = { 0 };
        __ldm_unused__ gpointer key = NULL;
        LdmModalias *modalias = NULL;

        g_hash_table_iter_init(&iter, self->modaliases);
        while (g_hash_table_iter_next(&iter, &key, (void **)&modalias)) {
                if (!ldm_modalias_matches_device(modalias, device)) {
                        continue;
                }

                ret = ldm_provider_new(plugin, device, ldm_modalias_get_package(modalias));
                ldm_provider_set_installed(ret, ldm_modalias_plugin_is_installed(modalias));
                return ret;
        }

        return NULL;
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
