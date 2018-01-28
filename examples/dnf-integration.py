#!/usr/bin/env python3
#
# This file is Public Domain and provided only for documentation purposes.
#
# Run : python3 dnf-integration.py
#
#

import gi
import os
gi.require_version('Ldm', '1.0')

import dnf.base

from gi.repository import Ldm


class DnfIntegration:

    dbo = None
    manager = None

    def __init__(self):
        self.build_dbo()
        self.manager = Ldm.Manager()
        self.build_providers()

        # Allow recieving hotplug events
        self.manager.connect('device-added', self.on_device_added)

    def build_dbo(self):
        """ Build our dnf configuration """
        conf = dnf.conf.Conf()
        conf.cachedir = "leCacheDir"
        conf.read('dnf.conf')
        base = dnf.Base(conf)

        # Set up the repo
        repo = dnf.repo.Repo("centOS", conf)
        repo.baseurl = "http://ftp.osuosl.org/pub/elrepo/elrepo/el7/x86_64"
        repo.gpgcheck = False
        repo.fasestmirror = True
        repo.deltarpm = False

        # Enable repo
        repo.enable()
        base.repos.add(repo)

        # Just needed for this demo because I'm not using rpmdb
        self.dbo = base
        self.sack = self.dbo.fill_sack(
            load_system_repo=False, load_available_repos=True)

    def build_providers(self):
        """ Iterate dnf package providers and populate storage with modaliases """
        query = self.sack.query()
        avail = query.available()
        for item in avail:
            if not item.provides:
                continue
            modalias_plugin = None

            for i in item.provides:
                it = str(i)
                if not it.startswith("modalias("):
                    continue
                it = it.split("=")[0].strip()
                subitem = it[9:len(it)-1]
                if not modalias_plugin:
                    modalias_plugin = Ldm.ModaliasPlugin.new(item.name)
                # TODO: Extract actual module name
                modalias_plugin.add_modalias(
                    Ldm.Modalias.new(subitem, item.name, item.name))

            # If we built a modalias plugin, add to manager
            if modalias_plugin:
                self.manager.add_plugin(modalias_plugin)

    def on_device_added(self, manager, device):
        """ Try finding a provider for newly added device """
        print("Device added: {}".format(device.get_name()))
        self.print_device(device)

    def enum_devices(self):
        """ Dispatch all devices through pretty printer """
        for device in self.manager.get_devices(Ldm.DeviceType.ANY):
            self.print_device(device)

    def print_device(self, device):
        """ Handle pretty printing of a device and providers """
        providers = self.manager.get_providers(device)
        if not providers:
            return

        print("Device: {} {}".format(device.get_vendor(), device.get_name()))
        for provider in providers:
            print(" * Provider: {}".format(provider.get_package()))
        print()

    def gpu_print(self):
        """ Handle the GPU specifics """
        gpu_config = Ldm.GPUConfig.new(self.manager)
        if gpu_config.has_type(Ldm.GPUType.OPTIMUS):
            print("Found Optimus device")
        elif gpu_config.has_type(Ldm.GPUType.HYBRID):
            print("Found Hybrid GPU")
        elif gpu_config.has_type(Ldm.GPUType.COMPOSITE):
            print("Found composite GPU")
        else:
            print("Simple GPU configuration")

        detect = gpu_config.get_detection_device()
        self.print_device(detect)


def main():
    """ Main entry """
    if not os.path.exists("leTempLoggage"):
        os.mkdir("leTempLoggage")
    if not os.path.exists("leCacheDir"):
        os.mkdir("leCacheDir")

    test = DnfIntegration()
    test.enum_devices()
    test.gpu_print()


if __name__ == "__main__":
    main()
