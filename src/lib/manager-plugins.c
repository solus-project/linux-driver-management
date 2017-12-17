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

/* ldm_manager_add_modalias_plugin_for_path:
 * @path: The fully qualified ".modaliases" file path
 *
 * Add a modalias plugin to the manager for the given path.
 */
void ldm_manager_add_modalias_plugin_for_path(LdmManager *self, const gchar *path)
{
        LdmPlugin *plugin = NULL;
        const gchar *plugin_id = NULL;

        plugin = ldm_modalias_plugin_new_from_filename(path);
        plugin_id = ldm_plugin_get_name(plugin);
        if (g_hash_table_contains(self->plugins, plugin_id)) {
                g_message("replacing plugin '%s' with '%s'", plugin_id, path);
        } else {
                g_message("new modalias plugin: %s", plugin_id);
        }
        g_hash_table_replace(self->plugins, g_strdup(plugin_id), plugin);
}

/**
 * ldm_manager_get_providers:
 *
 * Walk the plugins and find all known providers for the given device,
 * if they can support it. The list should be freed by the end user
 * and all the elements should be freed with `g_object_unref`.
 *
 * Returns: (element-type Ldm.Provider) (transfer full): a list of all possible providers
 */
GList *ldm_manager_get_providers(LdmManager *self, LdmDevice *device)
{
        GList *ret = NULL;
        __ldm_unused__ gpointer k = NULL;
        LdmPlugin *plugin = NULL;
        GHashTableIter iter = { 0 };

        g_hash_table_iter_init(&iter, self->plugins);
        while (g_hash_table_iter_next(&iter, &k, (void **)&plugin)) {
                LdmProvider *provider = NULL;

                /* See if this plugin supports the device */
                provider = ldm_plugin_get_provider(plugin, device);
                if (provider) {
                        ret = g_list_append(ret, provider);
                }
        }

        /* Not yet possible until providers have a plugin reference
        if (ret) {
                goto done;
        }

        ret = g_list_sort(ret, ldm_manager_sort_by_plugin);

done:
        return ret;
        */

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
