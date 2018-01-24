#!/usr/bin/env python2
#
# This file is Public Domain and provided only for documentation purposes.
#
# Run : python2 ./find-bt.py
#
# Note: This will happily run with Python3 too, I just picked a common baseline
#

import gi
gi.require_version('Ldm', '1.0')
from gi.repository import Ldm, GObject

class BluezPlugin(Ldm.Plugin):
    """ Quick and dirty plugin to force bluetooth detection.
    """

    # Not really needed but good practice
    __gtype_name__ = "BluezPlugin"

    def __init__(self):
        Ldm.Plugin.__init__(self)

    def do_get_provider(self, device):
        if not device.has_type(Ldm.DeviceType.BLUETOOTH):
            return None
        # Construct new LdmProvider for this plugin + device,
        # setting the package/bundle name to "pretendy-package"
        return Ldm.Provider.new(self, device, "bluez-bundle")

def main():
    manager = Ldm.Manager()
    manager.add_plugin(BluezPlugin())

    # An alternative is just to see if the len(devices) is not empty.
    devices = manager.get_devices(Ldm.DeviceType.BLUETOOTH)
    providerset = [manager.get_providers(x) for x in devices if x.has_attribute(Ldm.DeviceAttribute.HOST)]
    for providers in providerset:
        device = providers[0].get_device()
        for provider in providers:
            print("Provider for {} ({} {}): {}".format(
                device.get_path(),
                device.get_vendor(),
                device.get_name(),
                provider.get_package()))

if __name__ == "__main__":
    main()
