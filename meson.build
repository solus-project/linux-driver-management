project(
    'linux-driver-management',
    ['c'],
    version: '0.1.0',
    license: [
        'LGPL-2.1',
    ],
    default_options: [
        'c_std=c11',
        'prefix=/usr',
        'sysconfdir=/etc',
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
path_mandir = join_paths(path_prefix, get_option('mandir'))
path_datadir = join_paths(path_prefix, get_option('datadir'))
path_bindir = join_paths(path_prefix, get_option('bindir'))

# Get configuration bits together
cdata = configuration_data()
cdata.set_quoted('PACKAGE_VERSION', meson.project_version())
cdata.set_quoted('PACKAGE_NAME', meson.project_name())
cdata.set_quoted('LIBDIR', join_paths(get_option('prefix'), get_option('libdir')))
if get_option('with-emul32') == true
    cdata.set('WITH_EMUL32', '1')
    cdata.set_quoted('EMUL32LIBDIR', get_option('emul32-libdir'))
endif

# Write config.h now
config_h = configure_file(
     configuration: cdata,
     output: 'config.h',
)
config_h_dir = include_directories('.')

dep_pci = dependency('libpci')

# Sort out compatibility stuff
with_gl_driver_switch_compat = get_option('with-gl-driver-switch-compat')
subdir('compat')

# Now go build the source
subdir('src')

report = [
    '    Build configuration:',
    '    ====================',
    '',
    '    prefix:                                 @0@'.format(path_prefix),
    '    datadir:                                @0@'.format(path_datadir),
    '    sysconfdir:                             @0@'.format(path_sysconfdir),
    '    mandir:                                 @0@'.format(path_mandir),
    '    bindir:                                 @0@'.format(path_bindir),
    '',
    '    Extra modules:',
    '    ==============',
    '',
    '    gl-driver-switch-compat:                @0@'.format(with_gl_driver_switch_compat),
]

# Output some stuff to validate the build config
message('\n\n\n' + '\n'.join(report) + '\n\n')
