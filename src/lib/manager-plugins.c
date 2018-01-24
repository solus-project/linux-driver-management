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

#include <glob.h>

#include "config.h"
#include "manager-private.h"
#include "plugin.h"

#include "plugins/modalias-plugin.h"

/**
 * ldm_manager_add_plugin:
 * @plugin: (transfer full): New plugin to add.
 *
 * Add a new plugin to the current set of plugins. The plugins are used to
 * provide automatic hardware detection capabilities to the #LdmManager
 * and provide the internal API required for #ldm_manager_get_providers to
 * work.
 */
void ldm_manager_add_plugin(LdmManager *self, LdmPlugin *plugin)
{
        const gchar *plugin_id = NULL;

        g_return_if_fail(self != NULL);
        g_return_if_fail(plugin != NULL);

        /* If a plugin id is unspecified, make it the class name */
        plugin_id = ldm_plugin_get_name(plugin);
        if (!plugin_id) {
                plugin_id = G_OBJECT_CLASS_NAME(LDM_PLUGIN_GET_CLASS(plugin));
        }

        if (g_hash_table_contains(self->plugins, plugin_id)) {
                g_debug("replacing plugin '%s'", plugin_id);
        } else {
                g_debug("new plugin: %s", plugin_id);
        }

        /* Handle pythonic apis with non floating references */
        g_hash_table_replace(self->plugins, g_strdup(plugin_id), g_object_ref_sink(plugin));
}

/**
 * ldm_manager_add_modalias_plugin_for_path:
 * @path: The fully qualified ".modaliases" file path
 *
 * Add a new #LdmModaliasPlugin to the manager for the given path. This is a convenience
 * wrapper around #ldm_modalias_plugin_new_from_filename and #ldm_manager_add_plugin.
 *
 * Note that newer modalias plugins have a higher priority than older plugins,
 * so you should add newest drivers last if you have multiple driver versions.
 * This is already taken care of by using the glob-based function
 * #ldm_manager_add_modalias_plugins_for_directory
 *
 * i.e. insert 380 driver AFTER 340.
 *
 * Returns: TRUE if a new plugin was added
 */
gboolean ldm_manager_add_modalias_plugin_for_path(LdmManager *self, const gchar *path)
{
        LdmPlugin *plugin = NULL;

        if (!g_file_test(path, G_FILE_TEST_EXISTS)) {
                return FALSE;
        }

        plugin = ldm_modalias_plugin_new_from_filename(path);

        /* Enforce priority based on insert order */
        ldm_plugin_set_priority(plugin, self->modalias_plugin_priority);
        ++self->modalias_plugin_priority;

        ldm_manager_add_plugin(self, plugin);

        return TRUE;
}

/**
 * ldm_manager_add_modalias_plugins_for_directory:
 * @directory: Path containing `*.modaliases` files
 *
 * Attempt to bulk-add #LdmModaliasPlugin objects from the given directory to
 * ensure preservation of sort order and ease of use.
 *
 * This function is used to add well known modalias paths to the plugin and
 * construct plugins used for hardware detection.
 *
 * Returns: TRUE if a new plugin was added
 */
gboolean ldm_manager_add_modalias_plugins_for_directory(LdmManager *self, const gchar *directory)
{
        g_autofree gchar *glob_path = NULL;
        glob_t glo = { 0 };
        gboolean ret = FALSE;

        glob_path = g_strdup_printf("%s%s*.modaliases", directory, G_DIR_SEPARATOR_S);

        if (glob(glob_path, 0, NULL, &glo) != 0) {
                goto cleanup;
        }

        if (glo.gl_pathc < 1) {
                goto cleanup;
        }

        for (size_t i = 0; i < glo.gl_pathc; i++) {
                if (ldm_manager_add_modalias_plugin_for_path(self, glo.gl_pathv[i])) {
                        ret = TRUE;
                }
        }

cleanup:
        globfree(&glo);
        return ret;
}

/**
 * ldm_manager_add_system_modalias_plugins:
 *
 * Attempt to add all modalias plugins directory from the modalias directory
 * set when the library was compiled.
 *
 * This is a convenience wrapper around #ldm_manager_add_modalias_plugins_for_directory.
 *
 * Returns: TRUE if any modalias plugins were added.
 */
gboolean ldm_manager_add_system_modalias_plugins(LdmManager *self)
{
        return ldm_manager_add_modalias_plugins_for_directory(self, MODALIAS_DIR);
}

static gint ldm_manager_sort_by_priority(gconstpointer a, gconstpointer b)
{
        gint prioA = ldm_plugin_get_priority(ldm_provider_get_plugin(*(LdmProvider **)a));
        gint prioB = ldm_plugin_get_priority(ldm_provider_get_plugin(*(LdmProvider **)b));

        return prioB - prioA;
}

/**
 * ldm_manager_get_providers:
 *
 * Walk the plugins and find all known providers for the given device,
 * if they can support it. The returned #GPtrArray will free all elements
 * when it itself is freed.
 *
 * Returns: (element-type Ldm.Provider) (transfer full): a list of all possible providers
 */
GPtrArray *ldm_manager_get_providers(LdmManager *self, LdmDevice *device)
{
        GPtrArray *ret = NULL;
        __ldm_unused__ gpointer k = NULL;
        LdmPlugin *plugin = NULL;
        GHashTableIter iter = { 0 };

        ret = g_ptr_array_new_with_free_func(g_object_unref);

        g_hash_table_iter_init(&iter, self->plugins);
        while (g_hash_table_iter_next(&iter, &k, (void **)&plugin)) {
                LdmProvider *provider = NULL;

                /* See if this plugin supports the device */
                provider = ldm_plugin_get_provider(plugin, device);
                if (!provider) {
                        continue;
                }

                g_ptr_array_add(ret, g_object_ref_sink(provider));
        }

        g_ptr_array_sort(ret, ldm_manager_sort_by_priority);

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
