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

#include "modalias.h"
#include "util.h"

struct _LdmModaliasClass {
        GObjectClass parent_class;
};

/*
 * LdmModalias
 *
 * An LdmModalias is a mapping from an `fnmatch` style modalias match to
 * the required package name and kernel module.
 *
 * This is used in hardware detection to determine which package provides
 * the required kernel modules for any given hardware.
 */
struct _LdmModalias {
        GObject parent;

        /* What do we match? */
        gchar *match;

        /* What kernel driver enables this? */
        gchar *driver;

        /* Who do we belong to? */
        gchar *package;
};

G_DEFINE_TYPE(LdmModalias, ldm_modalias, G_TYPE_OBJECT)

enum { PROP_MATCH = 1, PROP_DRIVER, PROP_PACKAGE, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void ldm_modalias_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec);

/**
 * ldm_modalias_dispose:
 *
 * Clean up a LdmModalias instance
 */
static void ldm_modalias_dispose(GObject *obj)
{
        LdmModalias *self = LDM_MODALIAS(obj);

        g_clear_pointer(&self->match, g_free);
        g_clear_pointer(&self->driver, g_free);
        g_clear_pointer(&self->package, g_free);

        G_OBJECT_CLASS(ldm_modalias_parent_class)->dispose(obj);
}

/**
 * ldm_modalias_class_init:
 *
 * Handle class initialisation
 */
static void ldm_modalias_class_init(LdmModaliasClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_modalias_dispose;
        obj_class->get_property = ldm_modalias_get_property;

        /**
         * LdmModalias::match
         *
         * The fnmatch style string that would create a match
         */
        obj_properties[PROP_MATCH] = g_param_spec_string("match",
                                                         "The modalias match",
                                                         "fnmatch style matching string",
                                                         NULL,
                                                         G_PARAM_READABLE);

        /**
         * LdmModalias::driver
         *
         * The associated kernel driver
         */
        obj_properties[PROP_DRIVER] =
            g_param_spec_string("driver",
                                "The driver name",
                                "Kernel driver associated with this modalias",
                                NULL,
                                G_PARAM_READABLE);

        /**
         * LdmModalias::package
         *
         * The package or bundle containing this driver match
         */
        obj_properties[PROP_PACKAGE] =
            g_param_spec_string("package",
                                "The package name",
                                "Package or bundle containing this match",
                                NULL,
                                G_PARAM_READABLE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_modalias_get_property(GObject *object, guint id, GValue *value, GParamSpec *spec)
{
        LdmModalias *self = LDM_MODALIAS(object);

        switch (id) {
        case PROP_MATCH:
                g_value_set_string(value, self->match);
                break;
        case PROP_DRIVER:
                g_value_set_string(value, self->driver);
                break;
        case PROP_PACKAGE:
                g_value_set_string(value, self->package);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
}

/**
 * ldm_modalias_init:
 *
 * Handle construction of the LdmModalias
 */
static void ldm_modalias_init(__ldm_unused__ LdmModalias *self)
{
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
