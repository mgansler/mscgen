Name:           mscgen
Version:        0.19 
Release:        1%{?dist}
Summary:        Message Sequence Chart rendering program 
Group:          Applications/Multimedia 
License:        GPLv2+
URL:            http://www.mcternan.me.uk/mscgen/
Source0:        http://www.mcternan.me.uk/mscgen/software/%{name}-src-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires:  gd-devel 

%description
Mscgen is a small program that parses textual Message Sequence Chart 
descriptions and produces PNG, SVG, EPS or server side image maps 
(ismaps) as the output.

%prep
%setup -q


%build
%configure --docdir=%{_defaultdocdir}/%{name}-%{version}
make %{?_smp_mflags}


%check
make check


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_defaultdocdir}/%{name}-%{version}/
%{_bindir}/mscgen
%{_mandir}/man1/mscgen.1.*

%changelog
* Wed Sep 15 2010 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.19-1
- Version bump following upstream release. 

* Tue Sep 07 2010 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.18-3
- Further Fedora packaging fixes to remove duplicate doc (bug #630754, comment 4) 

* Tue Sep 07 2010 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.18-2
- Fixes from Fedora packaging review (bug #630754).

* Thu Aug 25 2010 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.18-1
- Initial version.

