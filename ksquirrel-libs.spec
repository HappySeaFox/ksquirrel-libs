Name: ksquirrel-libs
Summary: Ksquirrel - image viewer for KDE 
Group: User Interface/Desktops 
Version: 0.6.0 
Release: pre2
Copyright: LGPL 
Source: %{name}-%{version}-%{release}.tar.bz2
URL: http://ksquirrel.sf.net 
Packager: - 
Vendor: Baryshev Dmitry aka Krasu <ksquirrel@tut.by> 
BuildRoot: %{_tmppath}/%{name}-%{version}-root 
BuildRequires: gcc, gcc-c++, gettext 
BuildRequires: libpng-devel, libjpeg-devel, libtiff-devel
Requires: libpng, libjpeg, libtiff

%define _libdir /usr/lib/ksquirrel-libs

%description 
Ksquirrel is an image viewer for KDE implemented using OpenGL. 
You should have your videocard specific drivers been installed.

* ksquirrel-libs is a set of image decoders for KSquirrel
* or other viewers

%prep 
%setup

CFLAGS="$RPM_OPT_FLAGS" CXXFLAGS="$RPM_OPT_FLAGS" ./configure --libdir=/usr/lib/ksquirrel-libs $LOCALFLAGS

%build 
%__make %{?_smp_mflags} 

%install 
[ "$RPM_BUILD_ROOT" != "" ] && rm -rf $RPM_BUILD_ROOT 
%makeinstall 

%clean 
[ "$RPM_BUILD_ROOT" != "" ] && rm -rf $RPM_BUILD_ROOT 

%files 
%defattr(-,root,root) 
%{_libdir}/* 

%changelog 
