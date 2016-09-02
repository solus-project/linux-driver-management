/*
 * This file is part of linux-driver-management.
 *
 * Copyright © 2016 Ikey Doherty
 *
 * linux-driver-management is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#pragma once

#define __ldm_inline__ __attribute__((always_inline))

/**
 * All symbols are hidden by default so must be explicitly be made public in
 * LDM to define the ABI
 */
#define __ldm_public__ __attribute__((visibility("default")))

/**
 * Helpful in development to suppress compiler warnings for known-unused vars
 */
#define __ldm_unused__ __attribute__((unused))

/**
 * Define a cleanup-attribute based autofree helper
 *
 * Used in sol, nica, cve tool, etc.
 */
#define DEF_AUTOFREE(N, C)                                                                         \
        static inline void _autofree_func_##N(void *p)                                             \
        {                                                                                          \
                if (p && *(N **)p) {                                                               \
                        C(*(N **)p);                                                               \
                        (*(void **)p) = NULL;                                                      \
                }                                                                                  \
        }

/**
 * Mark the variable as an autofree, assuming the type is known with DEF_AUTOFREE
 */
#define autofree(N) __attribute__((cleanup(_autofree_func_##N))) N

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
