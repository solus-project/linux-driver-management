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

typedef struct _LdmModalias LdmModalias;
typedef struct _LdmModaliasClass LdmModaliasClass;

#define LDM_TYPE_MODALIAS ldm_modalias_get_type()
#define LDM_MODALIAS(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_MODALIAS, LdmModalias))
#define LDM_IS_MODALIAS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_MODALIAS))
#define LDM_MODALIAS_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_MODALIAS, LdmModaliasClass))
#define LDM_IS_MODALIAS_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_MODALIAS))
#define LDM_MODALIAS_GET_CLASS(o)                                                                  \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_MODALIAS, LdmModaliasClass))

GType ldm_modalias_get_type(void);

/* API */

LdmModalias *ldm_modalias_new(const gchar *match, const gchar *driver, const char *package);
const gchar *ldm_modalias_get_driver(LdmModalias *modalias);
const gchar *ldm_modalias_get_match(LdmModalias *modalias);
const gchar *ldm_modalias_get_package(LdmModalias *modalias);
gboolean ldm_modalias_matches(LdmModalias *modalias, const gchar *match_string);
gboolean ldm_modalias_matches_device(LdmModalias *modalias, LdmDevice *match_device);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmModalias, g_object_unref)

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
