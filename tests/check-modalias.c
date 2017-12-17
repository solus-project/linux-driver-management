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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include "ldm-private.h"
#include "ldm.h"
#include "util.h"

/**
 * My NVIDIA GPU, useful for testing.
 */
#define NVIDIA_MODALIAS "pci:v000010DEd00001C60sv00001558sd000065A4bc03sc00i00"

#define GLX_MATCH "pci:v000010DEd00001C60sv*sd*bc03sc*i*"
#define GLX_NO_MATCH "pci:v000010DEd00001B84sv*sd*bc03sc*i*"

#define NV_MODALIAS_FILE TEST_DATA_ROOT "/nvidia-glx-driver.modaliases"

/**
 * Abuse the private API to construct a fake LdmDevice with the given name, vendor, and modalias
 *
 * This allows us to test the devices against various match mechanisms.
 */
static LdmDevice *create_fake_device(const char *name, const char *vendor, const char *modalias)
{
        LdmDevice *ret = NULL;

        ret = g_object_new(LDM_TYPE_DEVICE, NULL);
        ck_assert(ret != NULL);

        ret->id.name = g_strdup(name);
        ret->id.vendor = g_strdup(vendor);
        if (modalias) {
                ret->os.modalias = g_strdup(modalias);
        }
        /* Deliberately fakey sysfs path */
        ret->os.sysfs_path = g_strdup_printf("/fake/path/%s/%s", name, vendor);

        return ret;
}

/**
 * Very simple test, let's make sure that our basic modalias matching is
 * actually working.
 */
START_TEST(test_modalias_simple)
{
        g_autoptr(LdmModalias) should_match = NULL;
        g_autoptr(LdmModalias) shouldnt_match = NULL;

        should_match = ldm_modalias_new(GLX_MATCH, "nvidia", "nvidia-glx-driver");
        shouldnt_match = ldm_modalias_new(GLX_NO_MATCH, "nvidia", "nvidia-glx-driver");

        fail_if(!ldm_modalias_matches(should_match, NVIDIA_MODALIAS),
                "Failed to correctly match NVIDIA driver");
        fail_if(ldm_modalias_matches(shouldnt_match, NVIDIA_MODALIAS),
                "Second modalias should NOT match");
}
END_TEST

START_TEST(test_modalias_device)
{
        g_autoptr(LdmDevice) fake_device = NULL;
        g_autoptr(LdmModalias) should_match = NULL;
        g_autoptr(LdmModalias) shouldnt_match = NULL;

        should_match = ldm_modalias_new(GLX_MATCH, "nvidia", "nvidia-glx-driver");
        shouldnt_match = ldm_modalias_new(GLX_NO_MATCH, "nvidia", "nvidia-glx-driver");
        fake_device = create_fake_device("GTX 1060", "NVIDIA", NVIDIA_MODALIAS);

        fail_if(!ldm_modalias_matches_device(should_match, fake_device),
                "Failed to correctly match NVIDIA driver");
        fail_if(ldm_modalias_matches_device(shouldnt_match, fake_device),
                "Second modalias should NOT match");
}
END_TEST

/**
 * Test loading modalias driver from a modalias file
 */
START_TEST(test_modalias_file)
{
        g_autoptr(LdmPlugin) driver = NULL;
        const gchar *driver_name = NULL;

        driver = ldm_modalias_plugin_new_from_filename(NV_MODALIAS_FILE);
        fail_if(!driver, "Failed to construct driver from modalias file");

        driver_name = ldm_plugin_get_name(driver);
        fail_if(!g_str_equal(driver_name, "nvidia-glx-driver"), "Plugin name is invalid");
}
END_TEST

/**
 * Standard helper for running a test suite
 */
static int ldm_test_run(Suite *suite)
{
        SRunner *runner = NULL;
        int n_failed = 0;

        runner = srunner_create(suite);
        srunner_run_all(runner, CK_VERBOSE);
        n_failed = srunner_ntests_failed(runner);
        srunner_free(runner);

        return n_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

static Suite *test_create(void)
{
        Suite *s = NULL;
        TCase *tc = NULL;

        s = suite_create(__FILE__);
        tc = tcase_create(__FILE__);
        suite_add_tcase(s, tc);

        tcase_add_test(tc, test_modalias_simple);
        tcase_add_test(tc, test_modalias_device);
        tcase_add_test(tc, test_modalias_file);

        return s;
}

int main(__ldm_unused__ int argc, __ldm_unused__ char **argv)
{
        return ldm_test_run(test_create());
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
