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

#include <device.h>
#include <glx-manager.h>
#include <gpu-config.h>
#include <ldm-enums.h>
#include <manager.h>
#include <modalias.h>
#include <provider.h>

/* Specialised devices */
#include <dmi-device.h>
#include <pci-device.h>
#include <usb-device.h>

/* Plugin API */
#include <plugin.h>
#include <plugins/modalias-plugin.h>

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
