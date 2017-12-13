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

#include "driver.h"
#include "util.h"

struct _LdmDriverPrivate {
        gchar *name;
        gint priority;
};

G_DEFINE_TYPE(LdmDriver, ldm_driver, G_TYPE_OBJECT)

enum { PROP_NAME = 1, PROP_PRIORITY, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void ldm_driver_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec);
static void ldm_driver_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

/**
 * ldm_driver_dispose:
 *
 * Clean up a LdmDriver instance
 */
static void ldm_driver_dispose(GObject *obj)
{
        LdmDriver *self = LDM_DRIVER(obj);

        g_clear_pointer(&self->priv->name, g_free);

        G_OBJECT_CLASS(ldm_driver_parent_class)->dispose(obj);
}

/**
 * ldm_driver_class_init:
 *
 * Handle class initialisation
 */
static void ldm_driver_class_init(LdmDriverClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_driver_dispose;
        obj_class->get_property = ldm_driver_get_property;
        obj_class->set_property = ldm_driver_set_property;

        /**
         * LdmDriver::name
         *
         * The display name for this driver
         */
        obj_properties[PROP_NAME] = g_param_spec_string("name",
                                                        "The driver name",
                                                        "Name for this driver",
                                                        NULL,
                                                        G_PARAM_READWRITE);

        /**
         * LdmDriver::priority
         *
         * Priority for this driver implementation. This can be useful in
         * cases where multiple drivers match some hardware, and the implementation
         * can opt to use the highest priority driver (i.e. multiple nvidia providers)
         */
        obj_properties[PROP_PRIORITY] =
            g_param_spec_int("priority",
                             "Driver priority",
                             "Used to sort drivers to find the best match",
                             0,
                             1000,
                             0,
                             G_PARAM_READWRITE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_driver_set_property(GObject *object, guint id, const GValue *value,
                                    GParamSpec *spec)
{
        LdmDriver *self = LDM_DRIVER(object);

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

static void ldm_driver_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmDriver *self = LDM_DRIVER(object);

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
 * ldm_driver_init:
 *
 * Handle construction of the LdmDriver
 */
static void ldm_driver_init(LdmDriver *self)
{
        self->priv = ldm_driver_get_instance_private(self);
}

/**
 * ldm_driver_get_name:
 *
 * Get the name of this driver
 *
 * Returns: (transfer none): Name for this driver
 */
const gchar *ldm_driver_get_name(LdmDriver *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->priv->name;
}

/**
 * ldm_driver_set_name:
 * @name: (transfer none): New name for this driver
 *
 * Set the name of the driver
 */
void ldm_driver_set_name(LdmDriver *self, const gchar *name)
{
        g_return_if_fail(self != NULL);
        g_return_if_fail(name != NULL);
        g_object_set(self, "name", name, NULL);
}

/**
 * ldm_driver_get_priority:
 *
 * Get the set priority of this driver
 *
 * Returns: Driver priority.
 */
gint ldm_driver_get_priority(LdmDriver *self)
{
        g_return_val_if_fail(self != NULL, 0);
        return self->priv->priority;
}

/**
 * ldm_driver_set_priority:
 * @priority: New priority for the driver
 *
 * Set the driver priority, useful for sorting.
 */
void ldm_driver_set_priority(LdmDriver *self, gint priority)
{
        g_return_if_fail(self != NULL);
        g_object_set(self, "priority", priority, NULL);
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
