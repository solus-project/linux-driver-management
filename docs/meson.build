# Handles gtkdoc installation
gnome = import('gnome')

gnome.gtkdoc(
    meson.project_name(),
    main_sgml: '@0@.sgml'.format(meson.project_name()),
    src_dir: join_paths(meson.source_root(), 'src', 'lib'),
    install: true,
    scan_args: [
        '--ignore-headers=ldm-private.h util.h',
    ],
    gobject_typesfile : '@0@.types'.format(meson.project_name()),
    dependencies: link_libldm_main,
)
