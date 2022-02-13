Name: ksquirrel-libs
Summary: Ksquirrel - image viewer for KDE 
Group: User Interface/Desktops 
Version: 0.5.0 
Release: pre4
Copyright: LGPL 
Source: %{name}-%{version}-%{release}.tar.bz2
URL: http://ksquirrel.sf.net 
Packager: - 
Vendor: Baryshev Dmitry aka Krasu <ksquirrel@tut.by> 
BuildRoot: %{_tmppath}/%{name}-%{version}-root 
BuildRequires: gcc, gcc-c++, gettext 
BuildRequires: kdelibs-devel, libpng-devel, libjpeg-devel, libtiff3-devel
Requires: kdelibs, libpng, libjpeg, libtiff3

%define libdir /usr/lib/squirrel

%description 
Ksquirrel is an image viewer for KDE implemented using OpenGL. 
You should have your videocard specific drivers been installed.

* ksquirrel-libs is a set of image decoders for KSquirrel

%prep 
%setup

CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --libdir=%libdir

%build 

%__make

%install 
[ "$RPM_BUILD_ROOT" != "" ] && rm -rf $RPM_BUILD_ROOT 
%makeinstall libdir=%buildroot%libdir

%clean 
[ "$RPM_BUILD_ROOT" != "" ] && rm -rf $RPM_BUILD_ROOT 

%files 
%defattr(-,root,root) 
%{_libdir}/*

%changelog 
