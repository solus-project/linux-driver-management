/*
 * This file is Public Domain and provided only for documentation purposes.
 *
 * Compile: gcc -Wall -g2 `pkg-config --cflags --libs ldm-0.1` hotplug.c -o hotplug
 * Run    : ./hotplug
 */

#include <ldm.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * Dummy handler for device adds.
 *
 * Note, the device is valid for the duration of this callback.
 */
static void device_added(__attribute((unused)) LdmManager *manager, LdmDevice *device,
                         __attribute((unused)) gpointer v)
{
        g_message("New device added: %s %s",
                  ldm_device_get_vendor(device),
                  ldm_device_get_name(device));
}

/**
 * Dummy handler for device removal. 2nd removal will make us quit.
 *
 * Note, the device is valid for the duration of this callback. After which it will be nuked ™
 */
static void device_removed(__attribute__((unused)) LdmManager *manager, LdmDevice *device,
                           GMainLoop *loop)
{
        static int refcount = 0;
        ++refcount;

        g_message("Device removed: %s %s",
                  ldm_device_get_vendor(device),
                  ldm_device_get_name(device));
        if (refcount == 2) {
                g_message("Second device removed, quitting!");
                g_main_loop_quit(loop);
        } else {
                g_message("Re-plug another device to stop the loop");
        }
}

int main(int argc, char **argv)
{
        g_autoptr(LdmManager) manager = NULL;
        g_autoptr(GMainLoop) loop = NULL;

        loop = g_main_loop_new(NULL, FALSE);

        /* Hook up basic signals */
        manager = ldm_manager_new(LDM_MANAGER_FLAGS_NONE);
        g_signal_connect(manager, "device-added", G_CALLBACK(device_added), NULL);
        g_signal_connect(manager, "device-removed", G_CALLBACK(device_removed), loop);

        g_message("Plug a device!");

        g_main_loop_run(loop);

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
