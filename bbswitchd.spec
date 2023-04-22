Name:           bbswitchd
Version:        0.1.3
Release:        2%{?dist}
Summary:        Daemon for toggling discrete NVIDIA GPU power on Optimus laptops

License:        GPLv3+
URL:            https://github.com/polter-rnd/bbswitchd
Source0:        https://github.com/polter-rnd/bbswitchd/archive/%{version}/bbswitchd-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.53.0
BuildRequires:  systemd
BuildRequires:  pkgconfig(libkmod)

Requires:       python3
Requires:       bbswitch-kmod
Requires(post):   dracut
Requires(postun): dracut
Recommends:     %{name}-selinux = %{version}-%{release}

%{?systemd_requires}


# UNIX socket group
%define socket_group bbswitchd
# Users from these will be added to socket group
%define admin_groups wheel


%description
Internally uses bbswitch kernel module to switch video adapter power state.

Useful for pre-Turing GPU generations without dynamic power management features,
allows to fully benefit from NVIDIA PRIME Render Offload technology
without need to keep graphics adapter turned on all the time.

%package selinux
Summary:        SELinux policies for bbswitchd

BuildRequires:  selinux-policy-devel

Requires: policycoreutils, libselinux-utils
Requires(post): selinux-policy-base >= %{selinux_policyver}, policycoreutils
Requires(postun): policycoreutils

BuildArch:      noarch

%description selinux
SELinux policy module for use with bbswitchd.


%prep
%autosetup -p1 -n bbswitchd-%{version}


%build
%meson -Dselinux=enabled
%meson_build


%install
%meson_install


%post
%systemd_post bbswitch-sleep.service
%systemd_post bbswitchd.service
%systemd_post bbswitchd.socket

if [ "${1}" -eq 1 ]; then
    # only add a group and members if the configured group does match the
    # default group and if the group is missing
    if grep -qx SocketGroup=%{socket_group} %{_unitdir}/bbswitchd.socket &&
        ! getent group %{socket_group} > /dev/null; then
        groupadd --system %{socket_group}
        users=$(getent -s files group %{admin_groups} | cut -d: -f4 | tr , '\n' | sort -u)
        for user in $users; do
            gpasswd -a $user %{socket_group}
        done
    fi

    systemctl start bbswitchd.service
fi

# Regenerate initramfs on install/upgrade
dracut -f

%preun
%systemd_preun bbswitch-sleep.service
%systemd_preun bbswitchd.service
%systemd_preun bbswitchd.socket


%postun
%systemd_postun bbswitch-sleep.service
%systemd_postun_with_restart bbswitchd.service
%systemd_postun_with_restart bbswitchd.socket

# Regenerate initramfs and remove group on uninstall
if [ "${1}" -eq 0 ]; then
    dracut -f
    groupdel %{socket_group} || true
fi

%post selinux
%selinux_modules_install %{_datadir}/selinux/packages/bbswitchd.pp


%postun selinux
if [ "${1}" -eq 0 ]; then
    %selinux_modules_uninstall bbswitchd
fi


%files
%license LICENSE
%{_bindir}/bbswitch
%{_sbindir}/bbswitchd
%{_prefix}/lib/modprobe.d/bbswitchd-blacklist.conf
%{_unitdir}/bbswitch-sleep.service
%{_unitdir}/bbswitchd.service
%{_unitdir}/bbswitchd.socket
%{_presetdir}/90-bbswitchd.preset
%{_udevrulesdir}/60-bbswitchd-nvidia-mutter.rules
%{_udevrulesdir}/90-bbswitchd-nvidia-dev.rules


%files selinux
%{_datadir}/selinux/packages/bbswitchd.pp
%{_datadir}/selinux/devel/include/contrib/bbswitchd.if


%changelog
* Sat Apr 22 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.3-2
- Restart bbswitchd.socket on update/reinstall

* Sat Apr 22 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.3-1
- Add bbswitch-sleep.service for toggling module on sleep/wakeup

* Sun Mar 19 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.2-2
- Don't remove bbswitchd group on update

* Sun Mar 19 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.2-1
- Minor fixes

* Wed Feb 08 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.1-3
- Added dracut to post/postun dependencies
- Fix SELinux dependency definitions

* Thu Jan 26 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.1-2
- Added Python3 to runtime dependencies

* Thu Jan 26 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.1-1
- Various packaging updates

* Tue Jan 17 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.0-1
- Initial release
