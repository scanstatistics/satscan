Summary:      ZZipLib - libZ-based ZIP-access Library
Name:         zziplib
Version:      0.10.27
Release:      %{_vendor}
Serial:       1
Copyright:    LGPL
Group:        Development/Libraries
URL:          http://zziplib.sf.net
Vendor:       Guido Draheim <guidod@gmx.de>
Source0:      http://prdownloads.sf.net/%{name}/%{name}-%{version}.tar.gz
BuildRoot:    /var/tmp/%{name}-%{version}-%{release}

Distribution: Original
Packager:     Guido Draheim <guidod@gmx.de>
Requires:     libz

#Begin3
# Author1:        too@iki.fi (Tomi Ollila)
# Author2:        guidod@gmx.de (Guido Draheim)
# Maintained-by:  guidod@gmx.de (Guido Draheim)
# Primary-Site:   zziplib.sf.net
# Keywords:       zip zlib inflate archive gamedata
# Platforms:      zlib posix
# Copying-Policy: Lesser GPL Version 2
#End

%package doc
Summary:      ZZipLib - Documentation Files
Group:        Development/Libraries

%package devel
Summary:      ZZipLib - Development Files
Group:        Development/Libraries
Requires:     zziplib

%package sfnet
Summary:      ZZipLib - SF.net htdocs
Group:        Documentation

%description
 : zziplib provides read access to zipped files in a zip-archive,
 : using compression based solely on free algorithms provided by zlib.
 zziplib provides an additional API to transparently access files
 being either real files or zipped files with the same filepath argument.
 This is handy to package many files being shared data into a single
 zip file - as it is sometimes used with gamedata or script repositories.
 The library itself is fully multithreaded, and it is namespace clean
 using the zzip_ prefix for its exports and declarations.
 
%description doc
 : zziplib provides read access to zipped files in a zip-archive,
 : using compression based solely on free algorithms provided by zlib.
 these are the (html) docs, mostly generated actually.

%description devel
 : zziplib provides read access to zipped files in a zip-archive,
 : using compression based solely on free algorithms provided by zlib.
 these are the header files needed to develop programs using zziplib.
 there are test binaries to hint usage of the library in user programs.

%description sfnet
 : zziplib provides read access to zipped files in a zip-archive,
 : using compression based solely on free algorithms provided by zlib.
 these are the files installed on the webserver for the project at
 http://zziplib.sourceforge.net

%prep
#'
%setup
CFLAGS="$RPM_OPT_FLAGS" sh configure --prefix=%{_prefix} --with-docdir=%{_docdir} TIMEOUT=9

%build
make
make doc 

%install
make install DESTDIR=%{buildroot}
make install-doc DESTDIR=%{buildroot}
make install-sfnet DESTDIR=%{buildroot}
cp ChangeLog %{buildroot}%{_docdir}/%{name}

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%clean
rm -rf %{buildroot}

%files
#    %defattr(-,root,root)
      %{_prefix}/lib/lib*

%files doc
%dir  %{_docdir}/%{name}
      %{_docdir}/%{name}/*

%files devel
#    %defattr(-,root,root)
      %{_prefix}/bin/*
      %{_prefix}/include/*

%files sfnet
      %{_datadir}/groups/*

%post doc
echo "Documentation install - please recreate package index"


