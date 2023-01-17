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

systemd_system_preset_dir = systemd.get_variable(
    pkgconfig : 'systemdsystempresetdir',
    pkgconfig_define : ['rootprefix', get_option('prefix')]
)

install_data('bbswitchd.preset',
    rename : '90-bbswitchd.preset',
    install_dir : systemd_system_preset_dir
)

install_data(['bbswitchd-nvidia-mutter.rules', 'bbswitchd-nvidia-dev.rules'],
    rename : ['60-bbswitchd-nvidia-mutter.rules', '90-bbswitchd-nvidia-dev.rules'],
    install_dir : get_option('prefix') / 'lib/udev/rules.d'
)

install_data('bbswitchd-blacklist.conf',
    install_dir : get_option('prefix') / 'lib/modprobe.d'
)
