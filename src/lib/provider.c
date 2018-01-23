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

#include "provider.h"
#include "util.h"

/**
 * SECTION:provider
 * @Short_description: Hardware enabling information
 * @see_also: #LdmDevice, #LdmManager
 * @Title: LdmProvider
 *
 * An LdmProvider is the result type when searching for hardware providers
 * using the #LdmManager plugins.
 */

struct _LdmProviderClass {
        GInitiallyUnownedClass parent_class;
};

struct _LdmProvider {
        GInitiallyUnowned parent;

        LdmDevice *device;
        LdmPlugin *plugin;
        gchar *package;
        gboolean installed;
};

static void ldm_provider_set_property(GObject *object, guint id, const GValue *value,
                                      GParamSpec *spec);
static void ldm_provider_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

G_DEFINE_TYPE(LdmProvider, ldm_provider, G_TYPE_INITIALLY_UNOWNED)

/* Property IDs */
enum { PROP_DEVICE = 1, PROP_PLUGIN, PROP_PACKAGE, PROP_INSTALLED, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

/**
 * ldm_provider_dispose:
 *
 * Clean up a LdmProvider instance
 */
static void ldm_provider_dispose(GObject *obj)
{
        LdmProvider *self = LDM_PROVIDER(obj);
        g_clear_pointer(&self->package, g_free);

        G_OBJECT_CLASS(ldm_provider_parent_class)->dispose(obj);
}

/**
 * ldm_provider_class_init:
 *
 * Handle class initialisation
 */
static void ldm_provider_class_init(LdmProviderClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_provider_dispose;
        obj_class->get_property = ldm_provider_get_property;
        obj_class->set_property = ldm_provider_set_property;

        /**
         * LdmProvider:device: (type LdmDevice) (transfer none)
         *
         * Device associated with this #LdmProvider
         */
        obj_properties[PROP_DEVICE] =
            g_param_spec_pointer("device",
                                 "Detection device",
                                 "Associated detection device",
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmProvider:plugin: (type LdmPlugin) (transfer none)
         *
         * Plugin associated with this #LdmProvider. This is the #LdmPlugin
         * instance responsible for creating this provider, and provided the
         * initial detection routine.
         */
        obj_properties[PROP_PLUGIN] =
            g_param_spec_pointer("plugin",
                                 "Detection plugin",
                                 "Parent plugin for this provider",
                                 G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmProvider:package: (transfer none)
         *
         * The package or bundle name required to install support for this
         * provider.
         */
        obj_properties[PROP_PACKAGE] =
            g_param_spec_string("package",
                                "Package or bundle name",
                                "Package or bundle name",
                                NULL,
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmProvider:installed
         *
         * Whether or not the provider is installed
         */
        obj_properties[PROP_INSTALLED] = g_param_spec_boolean("installed",
                                                              "Installed",
                                                              "Installation status",
                                                              FALSE,
                                                              G_PARAM_READWRITE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_provider_set_property(GObject *object, guint id, const GValue *value,
                                      GParamSpec *spec)
{
        LdmProvider *self = LDM_PROVIDER(object);

        switch (id) {
        case PROP_DEVICE:
                self->device = g_value_get_pointer(value);
                break;
        case PROP_PLUGIN:
                self->plugin = g_value_get_pointer(value);
                break;
        case PROP_PACKAGE:
                g_clear_pointer(&self->package, g_free);
                self->package = g_value_dup_string(value);
                break;
        case PROP_INSTALLED:
                self->installed = g_value_get_boolean(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

static void ldm_provider_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmProvider *self = LDM_PROVIDER(object);

        switch (id) {
        case PROP_DEVICE:
                g_value_set_pointer(value, self->device);
                break;
        case PROP_PLUGIN:
                g_value_set_pointer(value, self->plugin);
                break;
        case PROP_PACKAGE:
                g_value_set_string(value, self->package);
                break;
        case PROP_INSTALLED:
                g_value_set_boolean(value, self->installed);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * ldm_provider_init:
 *
 * Handle construction of the LdmProvider
 */
static void ldm_provider_init(__ldm_unused__ LdmProvider *self)
{
}

/**
 * ldm_provider_new:
 * @parent_plugin: The plugin to associate this provider with
 * @device: The device to associate this provider with
 * @package_name: The package or bundle name to install.
 *
 * Construct a new #LdmProvider with the given plugin and device for further
 * processing by the library end user.
 *
 * Returns: (transfer full): A new #LdmProvider instance
 */
LdmProvider *ldm_provider_new(LdmPlugin *parent_plugin, LdmDevice *device,
                              const gchar *package_name)
{
        return g_object_new(LDM_TYPE_PROVIDER,
                            "plugin",
                            parent_plugin,
                            "device",
                            device,
                            "package",
                            package_name,
                            NULL);
}

/**
 * ldm_provider_get_device:
 *
 * Get the device for this particular provider instance. This device is
 * the one that was used by the #LdmProvider:plugin to construct this
 * particular #LdmProvider instance.
 *
 * Returns: (transfer none): The associated #LdmDevice
 */
LdmDevice *ldm_provider_get_device(LdmProvider *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return self->device;
}

/**
 * ldm_provider_get_plugin:
 *
 * Get the #LdmPlugin that constructed this provider instance.
 *
 * Returns: (transfer none): The parent #LdmPlugin
 */
LdmPlugin *ldm_provider_get_plugin(LdmProvider *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return self->plugin;
}

/**
 * ldm_provider_get_package:
 *
 * Get the package name required to install this provider.
 *
 * Returns: (transfer none): The package name.
 */
const gchar *ldm_provider_get_package(LdmProvider *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->package;
}

/**
 * ldm_provider_get_installed:
 *
 * Determine if the provider is known to be installed
 *
 * Returns: TRUE if the provider is already installed.
 */
gboolean ldm_provider_get_installed(LdmProvider *self)
{
        g_return_val_if_fail(self != NULL, FALSE);
        return self->installed;
}

/**
 * ldm_provider_set_installed:
 * @installed: TRUE if this provider is considered installed.
 *
 * Mark this provider as installed
 */
void ldm_provider_set_installed(LdmProvider *self, gboolean installed)
{
        g_return_if_fail(self != NULL);
        /* Allow property bindings to work */
        g_object_set(self, "installed", installed, NULL);
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
