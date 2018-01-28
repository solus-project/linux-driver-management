/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016-2018 Linux Driver Management Developers, Solus Project
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <fnmatch.h>

#include "modalias.h"
#include "util.h"

struct _LdmModaliasClass {
        GInitiallyUnownedClass parent_class;
};

/**
 * SECTION:modalias
 * @Short_description: Modalias matching
 * @see_also: #LdmDevice, #LdmModaliasPlugin
 * @Title: LdmModalias
 *
 * An LdmModalias is a mapping from an `fnmatch` style modalias match to
 * the required package name and kernel module.
 *
 * This is used in hardware detection to determine which package provides
 * the required kernel modules for any given hardware. At the most simple
 * level, we attempt to match the #LdmModalias:match field, which is an
 * `fnmatch` style string, to each device's #LdmDevice:modalias field, which is
 * an explicit string set by the kernel.
 *
 * This allows for automatic hardware detection and association with each
 * #LdmPlugin.
 *
 * The primary use of #LdmModalias is by the #LdmModaliasPlugin implementation,
 * which adds a new modalias for every line in a file to allow hardware
 * matching.
 *
 * For example, if we're trying to match an NVIDIA GPU which has the following
 * modalias:
 *
 *      `pci:v000010DEd00001C60sv00001558sd000065A4bc03sc00i00`
 *
 * as set by the kernel, we might have a #LdmModalias:match line that looks like:
 *
 *      `pci:v000010DEd00001C60sv*sd*bc03sc*i*`
 *
 * Then all that is left is a simple comparison to check if the driver can be
 * supported by the given match. Once we verify a match, we also know from the
 * #LdmModalias:driver and #LdmModalias:package fields what the end user would
 * need to have installed on their system for the #LdmModalias:driver to be
 * activated. This helps immensely in detecting support for drivers.
 */
struct _LdmModalias {
        GInitiallyUnowned parent;

        /* What do we match? */
        gchar *match;

        /* What kernel driver enables this? */
        gchar *driver;

        /* Who do we belong to? */
        gchar *package;
};

G_DEFINE_TYPE(LdmModalias, ldm_modalias, G_TYPE_INITIALLY_UNOWNED)

enum { PROP_MATCH = 1, PROP_DRIVER, PROP_PACKAGE, N_PROPS };

static GParamSpec *obj_properties[N_PROPS] = {
        NULL,
};

static void ldm_modalias_set_property(GObject *object, guint id, const GValue *value,
                                      GParamSpec *spec);
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
        obj_class->set_property = ldm_modalias_set_property;

        /**
         * LdmModalias:match
         *
         * The fnmatch style string that would create a match
         */
        obj_properties[PROP_MATCH] =
            g_param_spec_string("match",
                                "The modalias match",
                                "fnmatch style matching string",
                                NULL,
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmModalias:driver
         *
         * The associated kernel driver
         */
        obj_properties[PROP_DRIVER] =
            g_param_spec_string("driver",
                                "The driver name",
                                "Kernel driver associated with this modalias",
                                NULL,
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        /**
         * LdmModalias:package
         *
         * The package or bundle containing this driver match
         */
        obj_properties[PROP_PACKAGE] =
            g_param_spec_string("package",
                                "The package name",
                                "Package or bundle containing this match",
                                NULL,
                                G_PARAM_CONSTRUCT_ONLY | G_PARAM_READWRITE);

        g_object_class_install_properties(obj_class, N_PROPS, obj_properties);
}

static void ldm_modalias_set_property(GObject *object, guint id, const GValue *value,
                                      GParamSpec *spec)
{
        LdmModalias *self = LDM_MODALIAS(object);

        switch (id) {
        case PROP_MATCH:
                g_clear_pointer(&self->match, g_free);
                self->match = g_value_dup_string(value);
                break;
        case PROP_DRIVER:
                g_clear_pointer(&self->driver, g_free);
                self->driver = g_value_dup_string(value);
                break;
        case PROP_PACKAGE:
                g_clear_pointer(&self->package, g_free);
                self->package = g_value_dup_string(value);
                break;
        default:
                G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, spec);
                break;
        }
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

/**
 * ldm_modalias_new:
 * @match: fnmatch style string to match hardware
 * @driver: Name of the driver for this modalias match
 * @package: Name of the package or bundle for this modalias match
 *
 * Returns: (transfer full): A newly initialised LdmModalias
 */
LdmModalias *ldm_modalias_new(const gchar *match, const gchar *driver, const char *package)
{
        return g_object_new(LDM_TYPE_MODALIAS,
                            "match",
                            match,
                            "driver",
                            driver,
                            "package",
                            package,
                            NULL);
}

/**
 * ldm_modalias_get_driver:
 *
 * This function will return the driver name (i.e "wl") associated with
 * this particular modalias match.
 *
 * Returns: (transfer none): The driver name
 */
const gchar *ldm_modalias_get_driver(LdmModalias *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->driver;
}

/**
 * ldm_modalias_get_match:
 *
 * This function will return the fnmatch-style match line used to check
 * if this #LdmModalias matches a given hardware device on the system.
 *
 * Returns: (transfer none): The match line
 */
const gchar *ldm_modalias_get_match(LdmModalias *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->match;
}

/**
 * ldm_modalias_get_package:
 *
 * This function will return the name of the package or bundle that contains
 * the corresponding driver required to enable hardware support.
 *
 * Returns: (transfer none): The package or bundle name.
 */
const gchar *ldm_modalias_get_package(LdmModalias *self)
{
        g_return_val_if_fail(self != NULL, NULL);
        return (const gchar *)self->package;
}

/**
 * ldm_modalias_matches:
 * @match_string: Device modalias to test against
 *
 * If the given match_string matches our own fnmatch style match, this function
 * will return true, indicating compatibility.
 *
 * Returns: True if the match_string is indeed a match
 */
gboolean ldm_modalias_matches(LdmModalias *self, const gchar *match_string)
{
        g_return_val_if_fail(self != NULL, FALSE);
        g_return_val_if_fail(self->match != NULL, FALSE);
        g_return_val_if_fail(match_string != NULL, FALSE);

        return fnmatch(self->match, match_string, 0) == 0 ? TRUE : FALSE;
}

/**
 * ldm_modalias_matches_device:
 * @match_device: An LdmDevice to test against
 *
 * This is a simple wrapper around #ldm_modalias_matches, and will simply pass
 * the device's #LdmDevice:modalias for testing.
 *
 * Returns: True if the match_device is indeed a match
 */
gboolean ldm_modalias_matches_device(LdmModalias *self, LdmDevice *match_device)
{
        g_return_val_if_fail(match_device != NULL, FALSE);
        g_autoptr(GList) kids = NULL;
        GList *elem = NULL;
        const gchar *id = NULL;

        /* Root match? */
        id = ldm_device_get_modalias(match_device);
        if (id && ldm_modalias_matches(self, id)) {
                return TRUE;
        }

        /* Any kids? */
        kids = ldm_device_get_children(match_device);
        if (!kids) {
                return FALSE;
        }

        /* Try matching child devices (interfaces) */
        for (elem = kids; elem; elem = elem->next) {
                LdmDevice *child_device = NULL;

                child_device = LDM_DEVICE(elem->data);
                if (ldm_modalias_matches_device(self, child_device)) {
                        return TRUE;
                }
        }

        return FALSE;
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
