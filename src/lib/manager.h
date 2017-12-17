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

typedef struct _LdmManager LdmManager;
typedef struct _LdmManagerClass LdmManagerClass;

/**
 * LdmManagerFlags
 * @LDM_MANAGER_FLAGS_NONE: No special behaviour required
 * @LDM_MANAGER_FLAGS_NO_MONITOR: Disable hotplug events
 *
 * Override the behaviour of the new LdmManager to allow disabling
 * of hotplug events, etc.
 */
typedef enum {
        LDM_MANAGER_FLAGS_NONE = 0,
        LDM_MANAGER_FLAGS_NO_MONITOR = 1 << 0,
} LdmManagerFlags;

#define LDM_TYPE_MANAGER ldm_manager_get_type()
#define LDM_MANAGER(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_MANAGER, LdmManager))
#define LDM_IS_MANAGER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_MANAGER))
#define LDM_MANAGER_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_MANAGER, LdmManagerClass))
#define LDM_IS_MANAGER_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_MANAGER))
#define LDM_MANAGER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_MANAGER, LdmManagerClass))

GType ldm_manager_get_type(void);

LdmManager *ldm_manager_new(LdmManagerFlags flags);
GList *ldm_manager_get_devices(LdmManager *manager, LdmDeviceType class_mask);
GList *ldm_manager_get_providers(LdmManager *manager, LdmDevice *device);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmManager, g_object_unref)

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
