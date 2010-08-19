Name:           mscgen
Version:        0.18 
Release:        1%{?dist}
Summary:        Message Sequence Chart rendering program 
Group:          Applications/Multimedia 
License:        GPLv2
URL:            http://www.mcternan.me.uk/mscgen/
Source0:        http://www.mcternan.me.uk/mscgen/software/%{name}-src-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  gd-devel 
Requires:       gd

%description
Mscgen is a small program that parses textual Message Sequence Chart 
descriptions and produces PNG, SVG, EPS or server side image maps 
(ismaps) as the output.

%prep
%setup -q


%build
%configure --docdir=%{_docdir}/%{name}-%{version}
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install-strip DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%doc %{_docdir}/%{name}-%{version}/COPYING
%doc %{_docdir}/%{name}-%{version}/README
%doc %{_docdir}/%{name}-%{version}/ChangeLog
%doc %{_docdir}/%{name}-%{version}/examples
%{_bindir}/mscgen
%{_mandir}/man1/mscgen.1.gz

%changelog
* Sun Aug 2 2009 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.17-1 
- Initial version.

