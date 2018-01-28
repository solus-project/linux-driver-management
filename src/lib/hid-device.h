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

G_BEGIN_DECLS

typedef struct _LdmHIDDevice LdmHIDDevice;
typedef struct _LdmHIDDeviceClass LdmHIDDeviceClass;

#define LDM_TYPE_HID_DEVICE ldm_hid_device_get_type()
#define LDM_HID_DEVICE(o) (G_TYPE_CHECK_INSTANCE_CAST((o), LDM_TYPE_HID_DEVICE, LdmHIDDevice))
#define LDM_IS_HID_DEVICE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), LDM_TYPE_HID_DEVICE))
#define LDM_HID_DEVICE_CLASS(o)                                                                    \
        (G_TYPE_CHECK_CLASS_CAST((o), LDM_TYPE_HID_DEVICE, LdmHIDDeviceClass))
#define LDM_IS_HID_DEVICE_CLASS(o) (G_TYPE_CHECK_CLASS_TYPE((o), LDM_TYPE_HID_DEVICE))
#define LDM_HID_DEVICE_GET_CLASS(o)                                                                \
        (G_TYPE_INSTANCE_GET_CLASS((o), LDM_TYPE_HID_DEVICE, LdmHIDDeviceClass))

GType ldm_hid_device_get_type(void);

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
