project(
    'linux-driver-management',
    ['c'],
    version: '1.0.1',
    license: [
        'LGPL-2.1',
    ],
    default_options: [
        'c_std=c11',
        'prefix=/usr',
        'sysconfdir=/etc',
        'localstatedir=/var',
    ],
)

am_cflags = [
    '-fstack-protector',
    '-Wall',
    '-pedantic',
    '-Wstrict-prototypes',
    '-Wundef',
    '-fno-common',
    '-Werror-implicit-function-declaration',
    '-Wformat',
    '-Wformat-security',
    '-Werror=format-security',
    '-Wconversion',
    '-Wunused-variable',
    '-Wunreachable-code',
    '-W',
]

# Add our main flags
add_global_arguments(am_cflags, language: 'c')

path_prefix = get_option('prefix')
path_sysconfdir = join_paths(path_prefix, get_option('sysconfdir'))
path_libdir = join_paths(path_prefix, get_option('libdir'))
path_mandir = join_paths(path_prefix, get_option('mandir'))
path_datadir = join_paths(path_prefix, get_option('datadir'))
path_bindir = join_paths(path_prefix, get_option('bindir'))
path_vardir = join_paths(path_prefix, get_option('localstatedir'), 'lib', meson.project_name())

# For stateless distros this is changed to /usr/share/xdg/autostart
path_autostartdir = get_option('with-autostart-dir')
if path_autostartdir == ''
    path_autostartdir = join_paths(path_sysconfdir, 'xdg', 'autostart')
endif

warn_modalias = false
with_modalias_dir = get_option('with-modalias-dir')
if with_modalias_dir != ''
    path_modalias_dir = join_paths(path_prefix, with_modalias_dir)
else
    path_modalias_dir = join_paths(path_datadir, meson.project_name(), 'modaliases')
    warn_modalias = true
endif

# Find out where xorg modules are installed to
dep_xorg = dependency('xorg-server', required: false)
if dep_xorg.found()
    xorg_module_dir = dep_xorg.get_pkgconfig_variable('moduledir')
else
    xorg_module_dir = join_paths(path_libdir, 'xorg', 'modules')
    message('Warning: Guessing xorg module directory! @0@'.format(xorg_module_dir))
    message('Rebuild with pkgconfig(xorg-server) to ensure real path is used')
endif

# Get configuration bits together
cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('PACKAGE_NAME', meson.project_name())
cdata.set_quoted('LIBDIR', path_libdir)
cdata.set_quoted('SYSCONFDIR', path_sysconfdir)
cdata.set_quoted('XORG_MODULE_DIRECTORY', xorg_module_dir)
cdata.set_quoted('MODALIAS_DIR', path_modalias_dir)

# Track dirs
cdata.set_quoted('LDM_TRACK_DIR', path_vardir)
with_hybrid_file = join_paths(path_vardir, 'hybrid') 
cdata.set_quoted('LDM_HYBRID_FILE', with_hybrid_file)

# Write config.h now
config_h = configure_file(
     configuration: cdata,
     output: 'config.h',
)
config_h_dir = include_directories('.')

# Required dependencies
dep_usb = dependency('libusb-1.0', version: '1.0.21')

glib_min_version = '>= 2.54.0'
dep_glib2 = dependency('glib-2.0', version: glib_min_version)
dep_gobject = dependency('gobject-2.0', version: glib_min_version)
dep_udev = dependency('libudev', version: '>= 215')

with_tests = get_option('with-tests')
enable_tests = false
if with_tests != 'no'
    required_test = false
    if with_tests == 'yes'
        required_test = true
    endif

    dep_check = dependency('check', version: '>= 0.12.0', required: required_test)
    dep_umockdev = dependency('umockdev-1.0', version: '>= 0.9.0', required: required_test)
    run_umockdev = find_program('umockdev-wrapper', required: required_test)

    if dep_check.found() and dep_umockdev.found() and run_umockdev.found()
        enable_tests = true
    endif
endif

enable_vapigen = true
run_vapigen = find_program('vapigen', required: false)
if not run_vapigen.found()
    message('Disabling vala binding generation as vapigen is not installed')
    enable_vapigen = false
endif

# If we have kmod, enable tooling
with_tools = get_option('with-tools')
enable_tools = false
if with_tools != 'no'
    required_tools = false
    if with_tools == 'yes'
        required_tools = true
    endif

    dep_kmod = dependency('libkmod', version: '>= 24', required: required_tools)

    if dep_kmod.found()
        enable_tools = true
    endif
endif

# Sort out compatibility stuff
with_gl_driver_switch_compat = get_option('with-gl-driver-switch-compat')
with_docs = get_option('with-docs')

# Define our ABI
abi_major_version = '1'
abi_minor_version = '0'
abi_micro_version = '0'

abi_version = '@0@.@1@.@2@'.format(
    abi_major_version,
    abi_minor_version,
    abi_micro_version,
)

# Maybe build compat
subdir('compat')

# Always build data
subdir('data')

# Now go build the source
subdir('src')

# Maybe test, maybe not.
if enable_tests == true
    subdir('tests')
endif

if with_docs == true
    subdir('docs')
endif

subdir('man')

report = [
    '    Build configuration:',
    '    ====================',
    '',
    '    prefix:                                 @0@'.format(path_prefix),
    '    datadir:                                @0@'.format(path_datadir),
    '    sysconfdir:                             @0@'.format(path_sysconfdir),
    '    mandir:                                 @0@'.format(path_mandir),
    '    bindir:                                 @0@'.format(path_bindir),
    '    modaliasdir:                            @0@'.format(path_modalias_dir),
    '    xorg module directory:                  @0@'.format(xorg_module_dir),
    '    XDG autostart directory:                @0@'.format(path_autostartdir),
    '    status directory:                       @0@'.format(path_vardir),
    '',
    '    Extra modules:',
    '    ==============',
    '',
    '    gl-driver-switch-compat:                @0@'.format(with_gl_driver_switch_compat),
    '    tools:                                  @0@'.format(enable_tools),
    '    vala bindings:                          @0@'.format(enable_vapigen),
    '',
    '    enable tests:                           @0@'.format(enable_tests),
    '    enable documentation:                   @0@'.format(with_docs),
]

if warn_modalias == true
report += [
    '', '',
    '    Warning:',
    '    ========',
    '    default modalias directory selected. Verify this is correct.',
    '    The modalias directory contains modalias definitions required',
    '    for driver detection and should be provided by the driver',
    '    packages in your distribution.',
]
endif

# Output some stuff to validate the build config
message('\n\n\n' + '\n'.join(report) + '\n\n')
