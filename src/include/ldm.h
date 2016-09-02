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

typedef struct LdmManager LdmManager;

/**
 * Construct a new LdmManager
 */
LdmManager *ldm_manager_new(void);

/**
 * Free a previously allocated LdmManager
 */
void ldm_manager_free(LdmManager *manager);
