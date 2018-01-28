# linux-driver-management

> Goal: Let's reinvent PnP for Linux

Linux Driver Management provides a core library and some tooling to enable the quick and easy enumeration of system devices, and functionality to match devices to packages/drivers. This is designed to be as agnostic as feasible whilst supporting a wide range of device classes, to provide a building block for driver management and discovery in Linux distributions.

Through this library we aim to centralise driver management solutions for Linux, and allow all distributions a chance to improve the UX around driver/device installation through device/driver discovery, with hotplug capabilities. Users should be able to plug in that new device and have the corresponding drivers offered to them dynamically, without having to hunt through software centers and menus for driver tools. Additionally - users might not realise they **need** to install extra support, and will rightfully believe that it's "broken".

Linux Driver Management is a [Solus project](https://solus-project.com/)

![logo](https://build.solus-project.com/logo.png)

## Core Overview

Effectively LDM provides a core library and some tooling around it. The library can be used to provide automatic hardware detection utilities and driver matching, in the style of `.modaliases` files (or with custom plugins at runtime).

 - Device enumeration
 - Discoverability by capability (i.e. via type or subsystem and type)
 - Abstraction of device complexities (`usb_device` -> `usb_interface` -> `hid` visible as single toplevel device)
 - Hotplug support via `udev` - allows offering drivers when a device is plugged in.
 - Matching of drivers/packages to specific hardware through multiple classes
 - GL/X11 configuration for proprietary drivers/Optimus ("always on" support)
 - Abstraction of system GPU configuration to determine iGPU vs dGPU, hybrid/optimus identification, simple classification, etc.

## TODO

Currently LDM library does the minimum work needed to enumerate devices and access very basic properties, whilst providing fairly complex matching systems. However, there are some things left to do in future:

 - Expose udev/system properties of devices
 - Add APIs to deal with each individual device child type
 - Further expose `LdmWifiDevice`, etc.

In short the enumeration functions currently focus on detecting capabilities, and for the first release advanced introspection has been omitted.


## Library

The core library provides a GObject API to query and discover devices. The `LdmManager` type is the main library entry point and supports library-level plugins to extend the detection capabilities. 

Devices are abstracted into toplevel `LdmDevice` objects, which are usually a composite device hiding the ugly internal details of `sysfs` / `udev`. This allows searching for, and interaction with, toplevel `LdmUSBDevice` objects for example that have a composite "type" of all child nodes. With this capability we can deal with a device logically, such as a "Logitech Keyboard", which exposes the `HID` and `USB` capabilities. Internally the child node tree is used to allow complex device matching through the plugin implementation.

The library ships with GObject Introspection bindings and can be used from any GIR enabled language (such as Vala and Python).

Pythonic example:

```python
from gi.repository import Ldm

manager = Ldm.Manager()
printers = manager.get_devices(Ldm.DeviceType.USB | Ldm.DeviceType.PRINTER)
print(printers[0].get_name())  # "Some Printer"
providers = manager.get_providers(printers[0])
print(providers[0].get_package()) # some-printer-package
```

## Distro Integration

This section provides information on the specifics of distribution integration of LDM. Most of this is focused on X11 specifics.

Note: As a design decision to simplify LDM, we opted to support "modern" libGL management only, and not those requiring manual management of the `libGL.so*` links (as Solus used to have). Thus it is assumed that the end user can only have **one** variant of a proprietary driver installed, and not multiple conflicting version of the NVIDIA driver. It is also assumed that `mesa` libGL is always available.

For best results, we recommend a `libglvnd` enabled graphical architecture, with each major versioned NVIDIA driver having a conflict with each other to enforce mutual exclusion. Additionally it is well advised to integrate the Fedora `ModulePath` patches to `xorg-server` to allow specifying the private NVIDIA directory for the `libglx.so` extension, to remove conflicts with `xorg-server`'s implementation.

### Session Init

LDM ships with a set of session hooks for popular display managers such as `gdm`, `sddm` and `LightDM`. These hooks will run `ldm-session-init` for X11 sessions, and should always be run. When an Optimus enabled system is configured with LDM, the session hook is responsible for setting up `xrandr` to allow "always on" Optimus support.

### GLX Configuration

This is provided via the library (`LdmGLXManager`) and exposed via the CLI command `linux-driver-management configure gpu`. This is intended to be run by the distro's postinstall hook system to set up the X11 configuration. This has been chiefly designed in mind with static packages that provide the relevant snippets for X11 to find library paths (see the Fedora `ModulePath` patches to `xorg-server`). It is recommended to use a glvnd-enabled system with separation between the `libGL` links as `linux-driver-management` no longer provides libGL symlink management.

During configuration, LDM will remove invalid `/etc/X11/xorg.conf` files if they explicitly enable a driver (i.e. `Driver "nvidia"`). For proprietary drivers LDM will create `/etc/X11/xorg.conf.d/00-ldm.conf` to turn on the driver at boot.

This may be unnecessary for some distros that use `PrimaryGPU` style patches however it should still be enabled. See the Optimus section for more details on this.

### Optimus

On GLVND enabled systems, the modern NVIDIA proprietary driver is able to use the correct `libGL` depending on the screen and kernel drivers. Currently LDM will enable "always on" support for Optimus via the `00-ldm.conf` X11 snippet.

In future iterations of LDM, we will make it easier to "disable" the NVIDIA card without removing the drivers and needing to reboot, just a logout and login again. To do this LDM will require control over the X11 configuration and early session initialisation, which is why it is recommended to not make use of `PrimaryGPU` unconditional Optimus enabling in conjunction with LDM.

The next natural step after this toggle behaviour will be to introduce support for dynamically enabling the dGPU for specific workloads. This will only be effective if LDM is given absolute control over the driver enabling in X11.

### Driver Matching

The core form of driver matching is to use `*.modaliases` files. These are identical in syntax to the older Ubuntu Jockey style modalias files, and can be used in a drop in fashion. Notably these files provide matching currently for the following subsystems:

 - `hid`
 - `usb`
 - `pci`
 - `dmi`
 - `bluetooth`

Example `modalias` files can be found in the `tests/data/*.modaliases` set. Essentially they provide a `fnmatch(3)` style string to match each device node `modalias`, the name of the kernel driver, and the name of the package or bundle that the user would need to install to enable it. It is then up to the consumer of the APIs to do something with those providers.

These modaliases can be generated at package build time and split into subpackages, allowing the main driver tool/software center to depend on all `-modaliases` subpackages to provide drop-in hardware detection. The exact interpretation of the kernel driver and package/bundle name are left to the discretion of the library consumer in order to allow LDM to remain agnostic.

For open source kernel drivers with well defined modaliases, you can use the `mkmodaliases(1)` tool provided by LDM to automatically generate these files during your package build. Note however that for the NVIDIA Proprietary drivers this will lead to poor matching, a problem well understood by many distributions already. It is also recommended to use a custom modalias for the broadcom drivers due to a poor built-in loose match:

```
alias pci:v000014E4d*sv*sd*bc02sc80i* wl broadcom-sta
```

Whilst `.modaliases` files **typically** refer to kernel modules, they can be (ab)used in software centers and such to match devices to userspace packages that add capabilities, as more of a recommendation system. For example, matching `hid:` lines for a mouse and providing a custom userspace USB driver (`usbfs`).

With the default meson configuration, modaliases will be found in `/usr/share/linux-driver-management/modaliases`.


License
-------

Copyright © 2016-2018 Linux Driver Management Developers

`linux-driver-management` is available under the terms of the `LGPL-2.1`
