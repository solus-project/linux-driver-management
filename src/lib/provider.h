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

G_BEGIN_DECLS

typedef struct _LdmProvider LdmProvider;
typedef struct _LdmProviderClass LdmProviderClass;

#define LDM_TYPE_PROVIDER ldm_provider_get_type()
#define LDM_PROVIDER(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_PROVIDER, LdmProvider))
#define LDM_IS_PROVIDER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_PROVIDER))
#define LDM_PROVIDER_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_PROVIDER, LdmProviderClass))
#define LDM_IS_PROVIDER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_PROVIDER))
#define LDM_PROVIDER_GET_CLASS(o)                                                                  \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_PROVIDER, LdmProviderClass))

GType ldm_provider_get_type(void);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmProvider, g_object_unref)

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
