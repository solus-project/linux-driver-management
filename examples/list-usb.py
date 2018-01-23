#!/usr/bin/env python2
#
# This file is Public Domain and provided only for documentation purposes.
#
# Run : python2 ./list-usb.py
#
# Note: This will happily run with Python3 too, I just picked a common baseline
#

import gi
gi.require_version('Ldm', '0.1')
from gi.repository import Ldm, GObject

class PretendyPlugin(Ldm.Plugin):

    # Not really needed but good practice
    __gtype_name__ = "PretendyPlugin"

    def __init__(self):
        Ldm.Plugin.__init__(self)

    def do_get_provider(self, device):
        """ Demonstrate basic matching with custom plugins """
        if not device.has_type(Ldm.DeviceType.AUDIO):
            return None
        return Ldm.Provider.new(self, device, "pretendy-package")

def main():
    manager = Ldm.Manager()
    manager.add_plugin(PretendyPlugin())

    for device in manager.get_devices(Ldm.DeviceType.USB):
        # Use gobject properties or methods
        print("USB Device: {} {}".format(
              device.props.vendor,
              device.get_name()))

        if device.has_type(Ldm.DeviceType.HID):
            print("\tHID Device!")

        for provider in manager.get_providers(device):
            plugin = provider.get_plugin()
            print("\tSuggested package: {}".format(provider.get_package()))

if __name__ == "__main__":
    main()
