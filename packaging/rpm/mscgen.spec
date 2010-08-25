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
* Thu Aug 25 2010 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.18-1
- Fix bad strncat() use allowing possible overrun.
   Thanks to ensc42 and Neils for finding this.
- Fix multi-line labels causing multiple dividers ("---")
   Thanks to llucax for the report, issue #31.
- Add support for arrowless arcs and bi-directional arrows as requested
   by xmlscott and started by Niels.  Can now use arrows such as <->, <=>
   for bidirectional arrows, --, ==, .. etc... for arcs without arrows.
   Issue #29.
- Fix text rendering over the box edge in multiline labels.
   Add patch to honour linecolour in boxes, issue #33.
   Thanks to Neils for this patch.
- Add 'textbgcolour' and 'arctextbgcolour' attributes to set the
   background colour for text in an entity or on an arc.
   Add support for -X and X- arcs which indicate a lost message.
- Add support for a new 'arcskip' attribute which allows an arc to have
   an additional slant to show delays or similar.
- Change configure.ac to attempt to use gdlib-config and then pkg-config.
   This enables simpler configuration/build on platforms where gd hasn't
   been updated to supply a pkg-config file e.g. Cygwin.  Update README
   regarding building on Cygwin and progress the cygport.

* Sun Aug 2 2009 Michael McTernan <Michael.McTernan.2001@cs.bris.ac.uk> 0.17-1 
- Initial version.

