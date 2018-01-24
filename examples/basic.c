/*
 * This file is Public Domain and provided only for documentation purposes.
 *
 * Compile: gcc -Wall -g2 `pkg-config --cflags --libs ldm-0.1` basic.c -o basic
 * Run    : ./basic
 */

#include <ldm.h>
#include <stdio.h>
#include <stdlib.h>

static void print_device(LdmDevice *device)
{
        if (ldm_device_has_type(device, LDM_DEVICE_TYPE_AUDIO)) {
                printf("Found HID device at %s\n", ldm_device_get_path(device));
        }

        printf("Device: %s %s\n", ldm_device_get_vendor(device), ldm_device_get_name(device));
}

int main(int argc, char **argv)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(GPtrArray) devices = NULL;

        /* Construct with 0 for hotplug support */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NO_MONITOR);
        devices = ldm_manager_get_devices(manager, LDM_DEVICE_TYPE_ANY);

        for (guint i = 0; i < devices->len; i++) {
                print_device(devices->pdata[i]);
        }

        return EXIT_SUCCESS;
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
