%define hash %(git rev-parse --short=8 HEAD)
%define date %(date -d @$(git show -s --format=%ct %{hash}) +%Y%m%d)
%define tag 3

Name: libjapi
Version: 0.%{tag}.%{date}_%{hash}
Release: 1
Summary: Universal JSON API library
License: none
Source0: libjapi.tar.gz
BuildRequires: json-c-devel

%description
Universal JSON API library

%package doc
Version: 0.%{tag}.%{date}_%{hash}
Release: 1
Summary: libjapi documentation
BuildRequires: doxygen

%description doc
Build documentation for the libjapi

%prep
%setup -q -n libjapi

%build
mkdir build
cd build
%cmake3 ..
%make_build
make doc

%install
install -d %{buildroot}/%{_libdir}/libjapi/
install -d %{buildroot}/%{_docdir}/libjapi/
install -d %{buildroot}/%{_includedir}/libjapi/
cp -ra %{_builddir}/libjapi/build/*.so %{buildroot}/%{_libdir}/
cp -ra %{_builddir}/libjapi/build/*.so.1 %{buildroot}/%{_libdir}/
cp -ra %{_builddir}/libjapi/build/doc/ %{buildroot}/%{_docdir}/libjapi/
cp -ra %{_builddir}/libjapi/README.md %{buildroot}/%{_docdir}/libjapi/
cp -ra %{_builddir}/libjapi/include/* %{buildroot}/%{_includedir}/libjapi/

%files doc
%{_docdir}/libjapi

%files
%{_libdir}/libjapi.so
%{_libdir}/libjapi.so.1
%{_includedir}/libjapi

%changelog
* Wed Jun 19 2019 Deniz Armagan <Deniz.Armagan@iis.fraunhofer.de>
- initial package release
