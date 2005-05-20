Summary: Simple parsing library suitable for configuration files
Name: libsimpleparser
Version: 0.1.1
Release: 2
Copyright: GPL
Group: Applications/System
Source: http://radu.rendec.ines.ro/prj/libsimpleparser/%{name}-%{version}.tar.gz
URL: http://radu.rendec.ines.ro/prj/libsimpleparser/
Buildroot: %{_tmppath}/%{name}-root

%description
This library can be used to parse simple command-oriented language
files. The syntax is very similar to the one used by named.conf of
bind. Commands, arguments and brace-delimited blocks are the most
common entitied that build up the files.

A callback mechanism is used to trigger user-defined actions when
various events occur in the parsing machine.

%prep
%setup -q -n libsimpleparser

%build
make

%install
install -m 0644 -D simpleparser.h %{buildroot}%{_includedir}/simpleparser.h 
install -m 0755 -D libsimpleparser.so.0.1.1 %{buildroot}%{_libdir}/libsimpleparser.so.0.1.1

%clean
rm -rf %{buildroot}

%post
ldconfig

%postun
ldconfig

%files
%defattr(-, root, root)
%doc README
%{_libdir}/*
%{_includedir}/*

%changelog
* Thu Oct 03 2002 Radu Rendec
- Spec file creation

