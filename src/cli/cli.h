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

/**
 * A CLICommand lends a subcommand approach to the "UI", akin to git
 */
typedef struct CLICommand {
        const char *name;                   /**<Textual name of the command */
        const char *summary;                /**<Help summary for the command */
        int (*exec)(int argc, char **argv); /**<Main entry */
} CLICommand;

/**
 * Configure a given set of drivers on the system
 */
int ldm_cli_configure(int argc, char **argv);

/**
 * Main status command
 */
int ldm_cli_status(int argc, char **argv);

/**
 * Defined in main.c - version emit
 */
int ldm_cli_version(int argc, char **argv);

/**
 * Help command
 */
int ldm_cli_help(int argc, char **argv);

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
