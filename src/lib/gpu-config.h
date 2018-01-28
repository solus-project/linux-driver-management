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

#pragma once

#include <glib-object.h>
#include <manager.h>

G_BEGIN_DECLS

typedef struct _LdmGPUConfig LdmGPUConfig;
typedef struct _LdmGPUConfigClass LdmGPUConfigClass;

/**
 * LdmGPUType:
 * @LDM_GPU_TYPE_SIMPLE: Trivial GPU configuration
 * @LDM_GPU_TYPE_HYBRID: Hybrid graphics discovered (i.e. Optimus)
 * @LDM_GPU_TYPE_COMPOSITE: Composite graphics configuration (SLI/Crossfire)
 * @LDM_GPU_TYPE_OPTIMUS: NVIDIA Optimus configuration (hybrid GPU)
 * @LDM_GPU_TYPE_SLI: NVIDIA SLI configuration (multiple GPUs)
 * @LDM_GPU_TYPE_CROSSFIRE: AMD Crossfire configuration (multiple GPUs)
 *
 * A GPU configuration can only have one active state at the time of detection
 * as far as LDM is concerned. It is in most cases a simple configuration, i.e.
 * not hybrid or composite. Beyond that, we tag to refine the state.
 *
 * Hybrid refers to *both* AMD and NVIDIA hybrid GPU approaches, and simply
 * indicates we discovered a hybrid GPU configuration.
 *
 * Composite indicates we're dealing with Crossfire or SLI systems.
 *
 * A GPU configuration may have one or more state applied, i.e. it may be
 * a hybrid GPU system but we can further refine this by tagging it as an
 * Optimus system too.
 */
typedef enum {
        LDM_GPU_TYPE_SIMPLE = 0,
        LDM_GPU_TYPE_HYBRID = 1 << 0,
        LDM_GPU_TYPE_COMPOSITE = 1 << 1,
        LDM_GPU_TYPE_OPTIMUS = 1 << 2,
        LDM_GPU_TYPE_SLI = 1 << 3,
        LDM_GPU_TYPE_CROSSFIRE = 1 << 4,
        LDM_GPU_TYPE_MAX,
} LdmGPUType;

#define LDM_TYPE_GPU_CONFIG ldm_gpu_config_get_type()
#define LDM_GPU_CONFIG(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_GPU_CONFIG, LdmGPUConfig))
#define LDM_IS_GPU_CONFIG(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_GPU_CONFIG))
#define LDM_GPU_CONFIG_CLASS(o)                                                                    \
        (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_GPU_CONFIG, LdmGPUConfigClass))
#define LDM_IS_GPU_CONFIG_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_GPU_CONFIG))
#define LDM_GPU_CONFIG_GET_CLASS(o)                                                                \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_GPU_CONFIG, LdmGPUConfigClass))

GType ldm_gpu_config_get_type(void);

/* API */
LdmGPUConfig *ldm_gpu_config_new(LdmManager *manager);
LdmManager *ldm_gpu_config_get_manager(LdmGPUConfig *config);
guint ldm_gpu_config_count(LdmGPUConfig *config);
LdmGPUType ldm_gpu_config_get_gpu_type(LdmGPUConfig *config);
gboolean ldm_gpu_config_has_type(LdmGPUConfig *config, LdmGPUType mask);
LdmDevice *ldm_gpu_config_get_primary_device(LdmGPUConfig *config);
LdmDevice *ldm_gpu_config_get_secondary_device(LdmGPUConfig *config);
LdmDevice *ldm_gpu_config_get_detection_device(LdmGPUConfig *config);
GPtrArray *ldm_gpu_config_get_providers(LdmGPUConfig *config);

G_DEFINE_AUTOPTR_CLEANUP_FUNC(LdmGPUConfig, g_object_unref)

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
