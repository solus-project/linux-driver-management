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

#include <driver.h>

G_BEGIN_DECLS

typedef struct _LdmModaliasDriver LdmModaliasDriver;
typedef struct _LdmModaliasDriverClass LdmModaliasDriverClass;

/*
 * LdmModaliasDriver
 *
 * An LdmModaliasDriver is essentially a kind of plugin as far as LDM is concerned,
 * and is used to match hardware configurations to provide the potential for
 * automatic driver detection.
 *
 * The base LdmModaliasDriver implementation does nothing by itself, and must be
 * extended to be useful.
 */
struct _LdmModaliasDriver {
        LdmDriver parent;
};

#define LDM_TYPE_MODALIAS_DRIVER ldm_modalias_driver_get_type()
#define LDM_MODALIAS_DRIVER(o)                                                                     \
        (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_MODALIAS_DRIVER, LdmModaliasDriver))
#define LDM_IS_MODALIAS_DRIVER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_MODALIAS_DRIVER))
#define LDM_MODALIAS_DRIVER_CLASS(o)                                                               \
        (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_MODALIAS_DRIVER, LdmModaliasDriverClass))
#define LDM_IS_MODALIAS_DRIVER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_MODALIAS_DRIVER))
#define LDM_MODALIAS_DRIVER_GET_CLASS(o)                                                           \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_MODALIAS_DRIVER, LdmModaliasDriverClass))

GType ldm_modalias_driver_get_type(void);

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
