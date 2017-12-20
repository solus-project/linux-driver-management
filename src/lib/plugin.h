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

#pragma once

#include <glib-object.h>

#include <device.h>

G_BEGIN_DECLS

typedef struct _LdmPluginPrivate LdmPluginPrivate;
typedef struct _LdmPlugin LdmPlugin;
typedef struct _LdmPluginClass LdmPluginClass;

/* Fix circular references between Plugin and Provider */
#include <provider.h>

/**
 * LdmPluginClass:
 * @parent_class: The parent class
 * @get_provider: Virtual get_provider function
 */
struct _LdmPluginClass {
        GObjectClass parent_class;

        LdmProvider *(*get_provider)(LdmPlugin *plugin, LdmDevice *device);

        /*< private >*/
        gpointer padding[12];
};

struct _LdmPlugin {
        GObject parent;
        LdmPluginPrivate *priv;
};

#define LDM_TYPE_PLUGIN ldm_plugin_get_type()
#define LDM_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_PLUGIN, LdmPlugin))
#define LDM_IS_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_PLUGIN))
#define LDM_PLUGIN_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_PLUGIN, LdmPluginClass))
#define LDM_IS_PLUGIN_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_PLUGIN))
#define LDM_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_PLUGIN, LdmPluginClass))

GType ldm_plugin_get_type(void);

/* API */
const gchar *ldm_plugin_get_name(LdmPlugin *plugin);
void ldm_plugin_set_name(LdmPlugin *plugin, const gchar *name);
gint ldm_plugin_get_priority(LdmPlugin *plugin);
void ldm_plugin_set_priority(LdmPlugin *plugin, gint priority);

LdmProvider *ldm_plugin_get_provider(LdmPlugin *self, LdmDevice *device);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmPlugin, g_object_unref)

G_END_DECLS

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
