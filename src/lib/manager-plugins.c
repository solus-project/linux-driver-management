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

        plugin_id = ldm_plugin_get_name(plugin);
        if (g_hash_table_contains(self->plugins, plugin_id)) {
                g_message("replacing plugin '%s'", plugin_id);
        } else {
                g_message("new plugin: %s", plugin_id);
        }
        g_hash_table_replace(self->plugins, g_strdup(plugin_id), g_object_ref_sink(plugin));
}

/**
 * ldm_manager_add_modalias_plugin_for_path:
 * @path: The fully qualified ".modaliases" file path
 *
 * Add a new #LdmModaliasPlugin to the manager for the given path. This is a convenience
 * wrapper around #ldm_modalias_plugin_new_from_filename and #ldm_manager_add_plugin
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
        ldm_manager_add_plugin(self, plugin);

        return TRUE;
}

static gint ldm_manager_sort_by_priority(gconstpointer a, gconstpointer b)
{
        gint prioA = ldm_plugin_get_priority(ldm_provider_get_plugin(*(LdmProvider **)a));
        gint prioB = ldm_plugin_get_priority(ldm_provider_get_plugin(*(LdmProvider **)b));

        return prioA - prioB;
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
                g_ptr_array_add(ret, provider);
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
