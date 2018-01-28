if with_glx_configuration == true

    data_conf = configuration_data()
    data_conf.set('BINDIR', path_bindir)

    path_lightdm = join_paths(path_datadir, 'lightdm', 'lightdm.conf.d')
    configure_file(
        configuration: data_conf,
        input: '99-ldm.conf.in',
        output: '99-ldm.conf',
        install_dir: path_lightdm,
    )

    path_sddm = join_paths(path_datadir, 'sddm', 'scripts')
    configure_file(
        configuration: data_conf,
        input: 'Xsetup.in',
        output: 'Xsetup',
        install_dir: path_sddm,
    )

    paths_gdm = [
        join_paths(path_datadir, 'gdm', 'greeter', 'autostart'),
        path_autostartdir,
    ]

    foreach path : paths_gdm
        configure_file(
            configuration: data_conf,
            input: 'ldm-session-init.desktop.in',
            output: 'ldm-session-init.desktop',
            install_dir: path,
        )
    endforeach

endif