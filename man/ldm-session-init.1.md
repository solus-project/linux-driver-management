ldm-session-init(1) -- Conditionally activate hybrid GPU for X11
================================================================


## SYNOPSIS

`ldm-session-init`


## DESCRIPTION

`ldm-session-init` is unconditionally executed by the display manager
at the start of the X11 session to **maybe** handle bootstrap of the
hybrid GPU. If the hybrid GPU driver is not activated, or is missing
the main driver, this program will exit immediately. This is handled
very quickly so as not to introduce a login penalty for non hybrid
GPU users.

Currently this command only supports Optimus™ graphics that have been
correctly configured via `linux-driver-management(1)`. Once this has
been correctly established, the relevant `xrandr(1)` calls are made to
set up the primary output provider to allow the discrete GPU to function
as the "primary" graphics.

For users who do not have a display manager, you can safely place a call
to `ldm-session-init` in your `xinitrc` or equivalent.
   
## EXIT STATUS

On success, 0 is returned. A non-zero return code signals a failure.

## COPYRIGHT

 * Copyright © 2017-2018 Ikey Doherty, License: CC-BY-SA-3.0

## SEE ALSO

 * https://github.com/solus-project/linux-driver-management
 * `linux-driver-management(1)`

## NOTES

Creative Commons Attribution-ShareAlike 3.0 Unported

 * http://creativecommons.org/licenses/by-sa/3.0/
