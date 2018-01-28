linux-driver-management(1) -- Query and manage drivers
======================================================


## SYNOPSIS

`linux-driver-management [options?] [subcommand]`


## DESCRIPTION

`linux-driver-management` is the main introspection and control binary provided
with the `linux-driver-management` library and project. Primarily it is used to
query system devices and status, and also provides configuration options for
X11 for certain drivers.

## SUBCOMMANDS

The following subcommands are understood by `linux-driver-management(1)`.

`configure [gpu]`

    Attempt configuration of the GPU specific details for X11. For
    "simple" devices configurations, this will invariably just configure
    the main X11 configuration file.

    For hybrid GPU configurations (such as Optimus), when used in
    conjunction with the proprietary drivers, specialist support will
    be enabled to ensure that X11 sessions will set the primary output
    provider to the discrete GPU.

    The result is that `ldm-session-init` will be invoked at the
    start of the session by the display manager. This can be added
    to your `xinitrc` file if you are not using a display manager.

`version`

    Print the program version, and exit.

`help`

    Print the help message, displaying all supported options, and exit.

`status`

    List the GPU configuration and any devices with known providers.

## OPTIONS

The following options are applicable to `linux-driver-management(1)`.

 * `-v`, `--version`

   Print the linux-driver-management version and exit.

 * `-h`, `--help`

   Print the help message, displaying all supported options, and exit.
   
   
## EXIT STATUS

On success, 0 is returned. A non-zero return code signals a failure.

## COPYRIGHT

 * Copyright © 2017-2018 Linux Driver Management Developers, Solus Project, License: CC-BY-SA-3.0

## SEE ALSO

 * https://github.com/solus-project/linux-driver-management
 * `mkmodaliases(1)`
 * `ldm-session-init(1)`

## NOTES

Creative Commons Attribution-ShareAlike 3.0 Unported

 * http://creativecommons.org/licenses/by-sa/3.0/
