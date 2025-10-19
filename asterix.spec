Name:           asterix
Version:        2.8.9
Release:        1%{?dist}
Summary:        ASTERIX protocol decoder library and tools

License:        GPLv3+
URL:            https://github.com/montge/asterix
Source0:        %{name}-%{version}.tar.gz

BuildRequires:  cmake >= 3.12
BuildRequires:  gcc-c++
BuildRequires:  expat-devel >= 2.1
BuildRequires:  make

Requires:       expat >= 2.1

%description
ASTERIX (All Purpose STructured EUROCONTROL Surveillance Information
EXchange) is a data format used in Air Traffic Management (ATM) for
exchanging surveillance and other data between ATM systems.

This package contains the ASTERIX decoder library and command-line tools
for parsing ASTERIX data from files, network streams, and PCAP captures.

%package        libs
Summary:        ASTERIX protocol decoder shared library
Requires:       expat >= 2.1

%description    libs
ASTERIX shared library for decoding ASTERIX protocol data.

%package        devel
Summary:        Development files for ASTERIX library
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}
Requires:       expat-devel

%description    devel
Development files (headers, static library) for the ASTERIX decoder library.

%package        tools
Summary:        ASTERIX protocol decoder command-line tools
Requires:       %{name}-libs%{?_isa} = %{version}-%{release}

%description    tools
Command-line tools for parsing ASTERIX data from files, network streams
(multicast), and PCAP captures. Supports output in text, JSON, and XML formats.

%prep
%autosetup -n %{name}-%{version}

%build
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_STATIC_LIBS=ON \
    -DBUILD_EXECUTABLE=ON \
    -DBUILD_TESTING=OFF

%cmake_build

%install
%cmake_install

%check
# Tests are run in CI, skip during package build

%files libs
%license COPYING
%doc README.md
%{_libdir}/libasterix.so.*

%files devel
%{_includedir}/asterix/
%{_libdir}/libasterix.so
%{_libdir}/libasterix.a

%files tools
%{_bindir}/asterix
%{_datadir}/asterix/
%{_docdir}/asterix/

%changelog
* Sat Oct 19 2024 ASTERIX Maintainers <noreply@github.com> - 2.8.9-1
- New upstream release 2.8.9
- Achieve 92.2%% test coverage (DO-278A AL-3 compliance progress)
- Fix critical bug: Uninitialized pointers in XMLParser
- Add 560+ comprehensive unit tests with Google Test
- Update GitHub Actions workflows to latest versions
- Improve multi-platform packaging support

* Fri Oct 18 2024 ASTERIX Maintainers <noreply@github.com> - 2.8.8-1
- Initial RPM package release
- Support for RHEL 8, RHEL 9, CentOS Stream 8, CentOS Stream 9
- Support for Fedora 38, 39, 40
