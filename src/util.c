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

#include <errno.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/**
 * Adapted from code now in clr-boot-manager LGPL-2.1
 * https://github.com/ikeydoherty/clr-boot-manager/blob/master/src/lib/util.c
 */
char *string_printf(const char *s, ...)
{
        va_list va;
        va_start(va, s);
        char *p = NULL;
        if (vasprintf(&p, s, va) < 0) {
                fputs("string_printf: Out of memory\n", stderr);
                abort();
        }
        va_end(va);
        return p;
}

/**
 * Taken from nica (LGPL-2.1)
 * https://github.com/ikeydoherty/libnica/blob/master/src/files.c
 */
bool mkdir_p(const char *path, mode_t mode)
{
        autofree(char) *cl = NULL;
        char *cl_base = NULL;

        if (streq(path, ".") || streq(path, "/") || streq(path, "//")) {
                return true;
        }

        cl = strdup(path);
        cl_base = dirname(cl);

        if (!mkdir_p(cl_base, mode) && errno != EEXIST) {
                return false;
        }

        return !((mkdir(path, mode) < 0 && errno != EEXIST));
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
