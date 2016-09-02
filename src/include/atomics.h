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

#include <assert.h>
#include <stdatomic.h>
#include <stdlib.h>

#include "util.h"

/**
 * Valid unref function for an sol atomic
 */
typedef void (*ldm_atomic_free)(void *);

/**
 * Base type for atomic reference types
 *
 * Ensure the ldm_atomic_t is the first member in the struct to gain
 * "free" atomic refcounts. Always proxy by ldm_atomic_init after
 * construction
 */
typedef struct ldm_atomic_t {
        ldm_atomic_free dtor; /**<Deconstructor when ref count hits 0 */
        atomic_int ref_count; /**<Current ref count */
} ldm_atomic_t;

/**
 * Increase the refcount of a given object by one, atomically.
 *
 * @note: @v is assumed to be an ldm_atomic_t based struct
 * @returns a reference to the ref'd struct
 */
__ldm_inline__ static inline void *ldm_atomic_ref(void *v)
{
        ldm_atomic_t *t = v;
        if (!t) {
                return NULL;
        }
        int ref_count = atomic_fetch_add(&(t->ref_count), 1);
        assert(ref_count >= 0);
        return t;
}

/**
 * Initialise an atomic reference type to a refcount of 1
 *
 * @param dtor Deconstructor to call when the refcount hits 0
 * @returns A reference to the object
 */
__ldm_inline__ static inline void *ldm_atomic_init(ldm_atomic_t *atom, ldm_atomic_free dtor)
{
        assert(atom->dtor == NULL);
        assert(atomic_load(&(atom->ref_count)) < 1);
        atom->dtor = dtor;
        return ldm_atomic_ref(atom);
}

/**
 * Decrease the refcount for an object by 1.
 * @note When the refcount hits 0, the deconstructor is called. If this doesn't
 * exist then it is assumed to be a basic type, and @free is called.
 *
 * @returns A reference to the object if the refcount is higher than 0, or NULL
 */
__ldm_inline__ static inline void *ldm_atomic_unref(void *v)
{
        ldm_atomic_t *t = v;
        if (!t) {
                return NULL;
        }
        int ref_count = atomic_fetch_sub(&(t->ref_count), 1);
        assert(ref_count > 0);
        if (ref_count == 1) {
                if (t->dtor) {
                        t->dtor(t);
                } else {
                        free(t);
                }
                return NULL;
        }
        return t;
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
