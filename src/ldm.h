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

#include <stdbool.h>

#include "device.h"

typedef enum {
        LDM_GPU_SIMPLE = 1, /**<Trivial GPU configuration */
        LDM_GPU_SLI,        /**<NVIDIA SLI configuration */
        LDM_GPU_CROSSFIRE,  /**<AMD Crossfire */
        LDM_GPU_OPTIMUS,    /**<NVIDIA Optimus (intel+nvidia) */
        LDM_GPU_AMD_HYBRID, /**<AMD Hybrid (amd+amd(apu)/intel+amd) */
} LdmGPUType;

/**
 * Configuration describing the GPU(s) on the system.
 * LDM is interested in simple or complex configurations, i.e. a single
 * primary configured, or a hybrid GPU configuration.
 */
typedef struct LdmGPUConfig {
        LdmDevice *primary;   /**<Main GPU, which *may* be the iGPU */
        LdmDevice *secondary; /**<Secondary GPU which will be the dGPU for hybrid */
        LdmGPUType type;
} LdmGPUConfig;

/**
 * Construct (and detect) the GPU configuration from a given set of devices
 */
LdmGPUConfig *ldm_gpu_config_new(LdmDevice *devices);

/**
 * Free the previously allocated GPU configuration
 */
void ldm_gpu_config_free(LdmGPUConfig *self);

/**
 * Attempt to configure the system GPU(s)
 *
 * This may result in the configuration of Xorg, the display manager, and the
 * OpenGL providers.
 */
bool ldm_configure_gpu(void);

DEF_AUTOFREE(LdmGPUConfig, ldm_gpu_config_free)

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
