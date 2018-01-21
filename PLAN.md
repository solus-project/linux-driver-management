## Deployment plan

In order to effectively switch Solus to the new Linux Driver Management, we
will need to stage it in two major releases.

### Phase 1

#### GLVND

The first step is to integrate GLVND into Solus as the primary `libGL.so`
provider, which will vastly simplify the logic required in LDM. I've opted
to no longer support the old filesystem butchery as it is highly problematic
and non portable.

Package `libglvnd` in the repository, rebuild `mesalib` with glvnd support and have it add a hard dependency on the library.

#### NVIDIA Packages

All:

 - Move `libglx.so` into `/usr/lib64/nvidia/` and use Fedora patches for `Modulepath` in `OutputClass` match.

Main series:

 - Drop any glvnd conflicts (dispatch, etc.)
 - Move all remaining files back into `/usr/lib{64,32}/` and drop `glx-provider` entirely.

304/340 series:

 - Move `libGL/EGL` etc into `/usr/lib{64,32}/nvidia` and provide `/usr/share/ld.so.conf.d/00-nvidia.conf` to allow old `libGL` to take precedence over `glvnd`'s implementation.

X.Org Server:

 - Update to 1.19.6
 - Apply the v2 Fedora patches for the `OutputClass` option support.

#### linux-driver-management

Get an initial release out of LDM (without hotplug and such) that is able
to perform the relevant **Hybrid GPU Specific** configuration tasks.

In short, we'll now only need to manage the DM/X.Org config files from the
`LdmGLXManager` for forcing the initial `PrimaryGPU`, instead of performing
butchery at the filesystem level.

We'll want to **unconditionally** unset any Optimus specific tweaks during
the first stage, and then **maybe** turn them back on should we find that
Optimus is now enabled.

### Phase 2

At this point the first LDM replacement would be in the repos having now
shed its `gl-driver-switch` heritage, focusing instead on detection and
the enabling of Optimus configurations (with a focus on GLVND only!).

Now we can finish off the libraries and lock down the API, finish off the
hotplug components and demonstrate integration through the Solus Software
Center, and call it a day.
