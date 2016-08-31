linux-driver-management
=======================

Linux Driver Management (LDM) provides the core functionality required for integration of drivers
into modern Linux distributions. It provides centralised tooling for detection  and configuration
for drivers, with a specific focus currently on display drivers.

Linux Driver Management is a [Solus project](https://solus-project.com/)

![logo](https://build.solus-project.com/logo.png)


NOTE: LDM is currently under heavy development, and should not be used yet.

Basic core design
-----------------

The initial core design will focus solely on the detection API before we bring in vtable based
driver management for the agnostic distro-installation bits. Right now the focus is a sane API
for backend detection.


        LdmManager {
                /* private */
        }
        
        ldm_manager_get_devices(manager, CONSTRAINT);
        ldm_manager_get_all_devices(manager);

        LdmDevice {
                type: str
                vendor: str
        }

        /* Hybrid configurations merged internally */
        LdmHybridDevice {
                { LdmDevice .type = HYBRID }
                devices: LdmDevice[]
        }
        

License
-------

Copyright © 2016 Ikey Doherty <ikey@solus-project.com>
linux-driver-management is available under the terms of the `LGPL-2.1`
