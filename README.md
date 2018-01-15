linux-driver-management
=======================

Linux Driver Management (LDM) provides the core functionality required for integration of drivers
into modern Linux distributions. It provides centralised tooling for detection  and configuration
for drivers, with a specific focus currently on display drivers.

Linux Driver Management is a [Solus project](https://solus-project.com/)

![logo](https://build.solus-project.com/logo.png)

TODO
----

 - [ ] Add GPU driver identification ("you need nvidia-glx-driver")
 - [ ] Emit notification to launch preconfigured binary when actioned (solus-sc, etc.)
 - [ ] Restore `linux-driver-management configure gpu` behaviour
 - [ ] Add persistence to prevent spamming about the same devices
 - [ ] Stabilize library and soname

License
-------

Copyright © 2016-2018 Ikey Doherty <ikey@solus-project.com>

`linux-driver-management` is available under the terms of the `LGPL-2.1`
