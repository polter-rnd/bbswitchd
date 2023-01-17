systemd = dependency('systemd')
systemd_system_unit_dir = systemd.get_variable(
    pkgconfig : 'systemdsystemunitdir',
    pkgconfig_define : ['rootprefix', get_option('prefix')]
)

systemd_unit_conf = configuration_data()
systemd_unit_conf.set('SBINDIR', get_option('prefix') / get_option('sbindir'))

configure_file(
    configuration : systemd_unit_conf,
    input : 'bbswitchd.service.in',
    output : 'bbswitchd.service',
    install : true,
    install_dir : systemd_system_unit_dir,
)

install_data('bbswitchd.socket',
    install_dir : systemd_system_unit_dir
)

udev = dependency('udev')
udev_dir = udev.get_variable(
    pkgconfig : 'udevdir',
    pkgconfig_define : ['rootprefix', get_option('prefix')]
)

install_data(['99-bbswitchd-nvidia-dev.rules'],
    install_dir : udev_dir / 'rules.d'
)