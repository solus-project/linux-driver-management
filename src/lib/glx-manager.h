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

#include "gpu-config.h"

G_BEGIN_DECLS

typedef struct _LdmGLXManager LdmGLXManager;
typedef struct _LdmGLXManagerClass LdmGLXManagerClass;

#define LDM_TYPE_GLX_MANAGER ldm_glx_manager_get_type()
#define LDM_GLX_MANAGER(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_GLX_MANAGER, LdmGLXManager))
#define LDM_IS_GLX_MANAGER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_GLX_MANAGER))
#define LDM_GLX_MANAGER_CLASS(o)                                                                   \
        (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_GLX_MANAGER, LdmGLXManagerClass))
#define LDM_IS_GLX_MANAGER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_GLX_MANAGER))
#define LDM_GLX_MANAGER_GET_CLASS(o)                                                               \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_GLX_MANAGER, LdmGLXManagerClass))

GType ldm_glx_manager_get_type(void);

/* API */
LdmGLXManager *ldm_glx_manager_new(void);

gboolean ldm_glx_manager_apply_configuration(LdmGLXManager *manager, LdmGPUConfig *config);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmGLXManager, g_object_unref)

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
