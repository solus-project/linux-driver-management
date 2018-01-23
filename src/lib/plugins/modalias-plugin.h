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

#pragma once

#include <glib-object.h>

#include <modalias.h>
#include <plugin.h>

G_BEGIN_DECLS

typedef struct _LdmModaliasPlugin LdmModaliasPlugin;
typedef struct _LdmModaliasPluginClass LdmModaliasPluginClass;

#define LDM_TYPE_MODALIAS_PLUGIN ldm_modalias_plugin_get_type()
#define LDM_MODALIAS_PLUGIN(o)                                                                     \
        (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_MODALIAS_PLUGIN, LdmModaliasPlugin))
#define LDM_IS_MODALIAS_PLUGIN(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_MODALIAS_PLUGIN))
#define LDM_MODALIAS_PLUGIN_CLASS(o)                                                               \
        (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_MODALIAS_PLUGIN, LdmModaliasPluginClass))
#define LDM_IS_MODALIAS_PLUGIN_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_MODALIAS_PLUGIN))
#define LDM_MODALIAS_PLUGIN_GET_CLASS(o)                                                           \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_MODALIAS_PLUGIN, LdmModaliasPluginClass))

GType ldm_modalias_plugin_get_type(void);

/* API */

LdmPlugin *ldm_modalias_plugin_new(const gchar *name);
LdmPlugin *ldm_modalias_plugin_new_from_filename(const gchar *filename);

void ldm_modalias_plugin_add_modalias(LdmModaliasPlugin *driver, LdmModalias *modalias);

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
