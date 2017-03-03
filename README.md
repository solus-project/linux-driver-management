linux-driver-management
=======================

Linux Driver Management (LDM) provides the core functionality required for integration of drivers
into modern Linux distributions. It provides centralised tooling for detection  and configuration
for drivers, with a specific focus currently on display drivers.

Linux Driver Management is a [Solus project](https://solus-project.com/)

![logo](https://build.solus-project.com/logo.png)


NOTE: LDM is currently under heavy development, and should not be used yet.
Please also note that the name is only a *working name*. We're actively seeking a replacement and
when a suitable name is found, it'll be replaced. The working name was chosen so that the project
could be started quickly without further delays. If you have a suggestion, open an issue please :)


Rationale
---------

**Why another driver manager?**

It is true that there are existing driver managers for Linux, notably the old Jockey (now known as
Ubuntu Drivers Common) and `gpu-manager` elements of Ubuntu. However, these are virtually hardwired
to Ubuntu, with many explicit dependencies on `dpkg`, `dkms`, `multi-arch` packages, etc.

Frankly, they are non-portable, and have design constraints placed on them that make them impossible
(and honestly, highly unattractive) for porting to newer Linux distributions, notably, Solus.

Traditionally the driver management has been as simple as:

 * Find PCI IDs and match with driver package list
 * Install them, running post-installs:
    - Move .so's around on the filesystem to provide relevant X.Org butchery
    - Run utils like `nvidia-xconfig`

Currently, DoFlickey and `gl-driver-switch` in Solus are already capable of most of these operations.
However, they are **not** agnostic, or portable. Furthermore, the advent of more complicated hardware
configurations in recent years, i.e. hybrid switchable (demux) graphics has broken the traditional
flow, requiring specialised management and configuration. No generic solution exists for these now
quite common cases.

Long story short, in terms of the proprietary driver world, switchable graphics, and a clean, portable
solution, we're all in the same boat here. The intent is to create a core library for performing the
management and detection code common to all the current use cases, to give Linux distributions a
central point to focus on.

Once the first steps are complete, which will be accompanied in Solus with a replacement of the existing
`gl-driver-switch` and `DoFlicky` components, the next element will be the dealing of dynamic runtime
switching for hybrid GPUs. What is badly needed, is a **clean**, **efficient** solution, and unfortunately
the current offerings don't stand up in this regard (Bumblebee, Primus, etc). More importantly, the
future of these projects is always shrouded in doubt, with longstanding rumours of imminent death
and a lack of real git activity.

Solus welcomes contributions into this centralized driver management effort, so we can finally put
these common, long-standing issues, behind us, with a common platform and focus. It will also mean
that no more Jockey-forks will need to exist in the world, and that **all** distributions will have
a common platform with which to enable drivers for their users, if they so wish.

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

Copyright © 2016-2017 Ikey Doherty <ikey@solus-project.com>

`linux-driver-management` is available under the terms of the `LGPL-2.1`
