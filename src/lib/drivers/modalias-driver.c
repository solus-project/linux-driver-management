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

#include "modalias-driver.h"
#include "util.h"

struct _LdmModaliasDriverClass {
        LdmDriverClass parent_class;
};

/*
 * LdmModaliasDriver
 *
 * The LdmModaliasDriver extends the base #LdmDriver and adds modalias-based
 * hardware detection to it.
 */
struct _LdmModaliasDriver {
        LdmDriver parent;

        /* Our known modalias implementations */
        GHashTable *modaliases;
};

G_DEFINE_TYPE(LdmModaliasDriver, ldm_modalias_driver, LDM_TYPE_DRIVER)

/**
 * ldm_modalias_driver_dispose:
 *
 * Clean up a LdmModaliasDriver instance
 */
static void ldm_modalias_driver_dispose(GObject *obj)
{
        G_OBJECT_CLASS(ldm_modalias_driver_parent_class)->dispose(obj);
}

/**
 * ldm_modalias_driver_class_init:
 *
 * Handle class initialisation
 */
static void ldm_modalias_driver_class_init(LdmModaliasDriverClass *klazz)
{
        GObjectClass *obj_class = G_OBJECT_CLASS(klazz);

        /* gobject vtable hookup */
        obj_class->dispose = ldm_modalias_driver_dispose;
}

/**
 * ldm_modalias_driver_init:
 *
 * Handle construction of the LdmModaliasDriver
 */
static void ldm_modalias_driver_init(LdmModaliasDriver *self)
{
        /* Map name to modalias */
        self->modaliases = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
}

/**
 * ldm_modalias_driver_new:
 * @name: Name for this driver instance
 *
 * Create a new LdmDriver for modalias detection with the given name
 *
 * Returns: (transfer full): A newly initialised LdmModaliasDriver
 */
LdmDriver *ldm_modalias_driver_new(const gchar *name)
{
        return g_object_new(LDM_TYPE_MODALIAS_DRIVER, "name", name, "priority", 0, NULL);
}

/**
 * ldm_modalias_driver_new_from_file:
 * @filename: A valid GFile
 *
 * Create a new LdmDriver for modalias detection, which is seeded with all
 * the modalias definitions referenced in the given file.
 *
 * Returns: (transfer full): A newly initialised LdmModaliasDriver
 */
LdmDriver *ldm_modalias_driver_new_from_file(GFile *file)
{
        g_return_val_if_fail(file != NULL, NULL);

        if (!g_file_query_exists(file, NULL)) {
                return NULL;
        }

        g_warning("new_from_file not yet implemented fully");

        return ldm_modalias_driver_new(NULL);
}

/**
 * ldm_modalias_driver_new_from_filename:
 * @filename: Path to a modaliases file
 *
 * Create a new LdmDriver for modalias detection. The named file will be
 * opened and the resulting driver will be seeded from that file.
 *
 * Returns: (transfer full): A newly initialised LdmModaliasDriver
 */
LdmDriver *ldm_modalias_driver_new_from_filename(const gchar *filename)
{
        g_autoptr(GFile) file = NULL;

        file = g_file_new_for_path(filename);
        if (!file) {
                return NULL;
        }
        return ldm_modalias_driver_new_from_file(file);
}

/**
 * ldm_modalias_driver_add_modalias:
 * @modalias: (transfer full): Modalias object to add to the table
 *
 * Add a new modalias object to the driver table. This method will take a new
 * reference to the modalias.
 */
void ldm_modalias_driver_add_modalias(LdmModaliasDriver *self, LdmModalias *modalias)
{
        const gchar *id = NULL;

        g_return_if_fail(self != NULL);
        g_return_if_fail(modalias != NULL);

        id = ldm_modalias_get_match(modalias);
        g_assert(id != NULL);

        g_hash_table_replace(self->modaliases, g_strdup(id), g_object_ref(modalias));
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
