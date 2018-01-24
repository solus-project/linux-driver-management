#!/usr/bin/env python2
#
# This file is Public Domain and provided only for documentation purposes.
#
# Run : python2 ./list-usb.py
#
# Note: This will happily run with Python3 too, I just picked a common baseline
#

import gi
gi.require_version('Ldm', '1.0')
from gi.repository import Ldm, GObject

class PretendyPlugin(Ldm.Plugin):
    """ PretendyPlugin extends the "normal" runtime support for plugins.
        By default LdmManager will not load and plugins, giving full control
        to the developer.

        It is usually more desirable to add modalias plugins but it is
        perfectly reasonable to add plugins that match on vendor + capability.

        Note in Python + GObject to override a virtual method, we prefix the
        name with "do_". Thus, "->get_provider" is "do_get_provider". It is
        also OK to return None (NULL) from the method if you don't support
        the device in your plugin.
    """

    # Not really needed but good practice
    __gtype_name__ = "PretendyPlugin"

    def __init__(self):
        Ldm.Plugin.__init__(self)

    def do_get_provider(self, device):
        """ Demonstrate basic matching with custom plugins """
        if not device.has_type(Ldm.DeviceType.AUDIO):
            return None

        # Construct new LdmProvider for this plugin + device,
        # setting the package/bundle name to "pretendy-package"
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
