mkmodaliases(1) -- Construct modaliases file for kernel modules
=============================================================


## SYNOPSIS

`mkmodaliases package-name [.ko file] [.ko file]`


## DESCRIPTION

`mkmodaliases` is a tool to generate `.modaliases` files used for hardware
detection when using linux-driver-management. These files contain a modalias
entry per line, defining the modalias pattern match and kernel module names.

These are used by the LDM library to provide automatic matching of hardware
devices to kernel modules.

Upon success, the modalias file is emitted to the stdout, unless the `-o` option
is provided to write to a specific file.

## OPTIONS

The following options are applicable to `mkmodaliases(1)`.


 * `-o`, `--output`

   Redirect the output to a named file, generating a modalias in that path
   instead of on the default stdout.
 
 * `-v`, `--version`

   Print the mkmodaliases version and exit.

 * `-h`, `--help`

   Print the help message, displaying all supported options, and exit.
   
   
## EXIT STATUS

On success, 0 is returned. A non-zero return code signals a failure.

## COPYRIGHT

 * Copyright © 2017-2018 Ikey Doherty, License: CC-BY-SA-3.0

## SEE ALSO

 * https://github.com/solus-project/linux-driver-management

## NOTES

Creative Commons Attribution-ShareAlike 3.0 Unported

 * http://creativecommons.org/licenses/by-sa/3.0/
