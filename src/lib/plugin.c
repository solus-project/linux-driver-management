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

#define _GNU_SOURCE

#include "plugin.h"
#include "util.h"

/**
 * SECTION:plugin
 * @Short_description: Find providers for the hardware
 * @see_also: #LdmDevice, #LdmModaliasPlugin
 * @Title: LdmPlugin
 *
 * An LdmPlugin is used to find potential software that the user should
 * have present or install to facilitate some hardware support.
 *
 * The base LdmPlugin implementation does nothing by itself, and must be
 * extended to be useful.
 */
struct _LdmPluginPrivate {
        gchar *name;
        gint priority;
};

G_DEFINE_TYPE_WITH_PRIVATE(LdmPlugin, ldm_plugin, G_TYPE_OBJECT)

enum { PROP_NAME = 1, PROP_PRIORITY, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void ldm_plugin_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec);
static void ldm_plugin_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

/**
 * ldm_plugin_dispose:
 *
 * Clean up a LdmPlugin instance
 */
static void ldm_plugin_dispose(GObject *obj)
{
        LdmPlugin *self = LDM_PLUGIN(obj);

        g_clear_pointer(&self->priv->name, g_free);

        G_OBJECT_CLASS(ldm_plugin_parent_class)->dispose(obj);
}

/**
 * ldm_plugin_class_init:
 *
 * Handle class initialisation
 */
static void ldm_plugin_class_init(LdmPluginClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_plugin_dispose;
        obj_class->get_property = ldm_plugin_get_property;
        obj_class->set_property = ldm_plugin_set_property;

        /**
         * LdmPlugin:name
         *
         * The display name for this plugin
         */
        obj_properties[PROP_NAME] = g_param_spec_string("name",
                                                        "The plugin name",
                                                        "Name for this plugin",
                                                        NULL,
                                                        G_PARAM_READWRITE);

        /**
         * LdmPlugin:priority
         *
         * Priority for this plugin implementation. This can be useful in
         * cases where multiple plugins match some hardware, and the implementation
         * can opt to use the highest priority plugin (i.e. multiple nvidia providers)
         */
        obj_properties[PROP_PRIORITY] =
            g_param_spec_int("priority",
                             "Plugin priority",
                             "Used to sort plugins to find the best match",
                             0,
                             1000,
                             0,
                             G_PARAM_READWRITE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_plugin_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec)
{
        LdmPlugin *self = LDM_PLUGIN(object);

        switch (id) {
        case PROP_NAME:
                g_clear_pointer(&self->priv->name, g_free);
                self->priv->name = g_value_dup_string(value);
                break;
        case PROP_PRIORITY:
                self->priv->priority = g_value_get_int(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void ldm_plugin_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmPlugin *self = LDM_PLUGIN(object);

        switch (id) {
        case PROP_NAME:
                g_value_set_string(value, self->priv->name);
                break;
        case PROP_PRIORITY:
                g_value_set_int(value, self->priv->priority);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * ldm_plugin_init:
 *
 * Handle construction of the LdmPlugin
 */
static void ldm_plugin_init(LdmPlugin *self)
{
        self->priv = ldm_plugin_get_instance_private(self);
}

/**
 * ldm_plugin_get_name:
 *
 * Get the name of this plugin
 *
 * Returns: (transfer none): Name for this plugin
 */
const gchar *ldm_plugin_get_name(LdmPlugin *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->priv->name;
}

/**
 * ldm_plugin_set_name:
 * @name: (transfer none): New name for this plugin
 *
 * Set the name of the plugin
 */
void ldm_plugin_set_name(LdmPlugin *self, const gchar *name)
{
        g_return_if_fail(self != NULL);
        g_return_if_fail(name != NULL);
        g_object_set(self, "name", name, NULL);
}

/**
 * ldm_plugin_get_priority:
 *
 * Get the set priority of this plugin
 *
 * Returns: Plugin priority.
 */
gint ldm_plugin_get_priority(LdmPlugin *self)
{
        g_return_val_if_fail(self != NULL, 0);
        return self->priv->priority;
}

/**
 * ldm_plugin_set_priority:
 * @priority: New priority for the plugin
 *
 * Set the plugin priority, useful for sorting.
 */
void ldm_plugin_set_priority(LdmPlugin *self, gint priority)
{
        g_return_if_fail(self != NULL);
        g_object_set(self, "priority", priority, NULL);
}

/**
 * ldm_plugin_get_provider:
 *
 * Virtual method that must be overridden by plugin implementations to provide
 * the required #LdmProvider solution for the given hardware.
 *
 * This may return NULL if the plugin doesn't support the given device.
 *
 * Returns: (transfer full) (nullable): A new #LdmProvider if possible
 */
LdmProvider *ldm_plugin_get_provider(LdmPlugin *self, LdmDevice *device)
{
        g_assert(self != NULL);
        g_return_val_if_fail(device != NULL, NULL);
        LdmPluginClass *klazz = LDM_PLUGIN_GET_CLASS(self);
        g_assert(klazz->get_provider != NULL);
        return klazz->get_provider(self, device);
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
