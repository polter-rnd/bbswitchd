fs = import('fs')
selinux_makefile = '/usr/share/selinux/devel/Makefile'
assert(fs.is_file(selinux_makefile), selinux_makefile + ' not found')

# Copy all SELinux policy sources to build directory
copy_gen = generator(find_program('cp'),
    output  : '@PLAINNAME@',
    arguments : ['@INPUT@', '@OUTPUT@']
)

custom_target('SELinux policy',
    input : copy_gen.process(
        'bbswitchd.te',
        'bbswitchd.if',
        'bbswitchd.fc'
    ),
    output : 'bbswitchd.pp',
    command : [
        'make', '-C', 'selinux', '-f', selinux_makefile
    ],
    build_by_default : true,
    install : true,
    install_dir : get_option('datadir') / 'selinux/packages',
)

install_data('bbswitchd.if',
    install_dir : get_option('datadir') / 'selinux/devel/include/contrib',
)