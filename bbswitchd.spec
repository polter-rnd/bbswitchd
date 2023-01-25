Name:           bbswitchd
Version:        0.1.0
Release:        2%{?dist}
Summary:        Daemon for toggling discrete NVIDIA GPU power on Optimus laptops

License:        GPLv3+
URL:            https://github.com/polter-rnd/bbswitchd
Source0:        https://github.com/polter-rnd/bbswitchd/archive/%{version}/bbswitchd-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.53.0
BuildRequires:  systemd
BuildRequires:  pkgconfig(libkmod)

Requires:       bbswitch-kmod
Recommends:     %{name}-selinux = %{version}-%{release}

%{?systemd_requires}


%description
Internally uses bbswitch kernel module to switch video adapter power state.

Useful for pre-Turing GPU generations without dynamic power management features,
allows to fully benefit from NVIDIA PRIME Render Offload technology
without need to keep graphics adapter turned on all the time.

%package selinux
Summary:        SELinux policies for bbswitchd

BuildRequires:  selinux-policy-devel

Requires(post): libselinux-utils
Requires(post): policycoreutils
Requires(post): policycoreutils-python-utils
Requires(post): selinux-policy-base >= %{_selinux_policy_version}

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


%pre
getent group bbswitchd >/dev/null || groupadd -r bbswitchd


%post
%systemd_post bbswitchd.service
%systemd_post bbswitchd.socket

systemctl daemon-reload
if [ $1 -eq 2 ]; then
    systemctl try-restart bbswitchd.service
else
    systemctl start bbswitchd.service
fi


%preun
%systemd_preun bbswitchd.service
%systemd_preun bbswitchd.socket


%postun
%systemd_postun bbswitchd.service
%systemd_postun bbswitchd.socket


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
%{_unitdir}/bbswitchd.service
%{_unitdir}/bbswitchd.socket
%{_presetdir}/90-bbswitchd.preset
%{_udevrulesdir}/60-bbswitchd-nvidia-mutter.rules
%{_udevrulesdir}/90-bbswitchd-nvidia-dev.rules


%files selinux
%{_datadir}/selinux/packages/bbswitchd.pp
%{_datadir}/selinux/devel/include/contrib/bbswitchd.if


%changelog
* Tue Jan 17 2023 Pavel Artsishevsky <polter.rnd@gmail.com> - 0.1.0-1
- Initial release
