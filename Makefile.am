SUBDIRS = kernel doc

AUTOMAKE_OPTIONS = foreign 

pkgconfigdir = ${libdir}/pkgconfig
pkgconfig_DATA = ksquirrellibs.pc

$(top_srcdir)/subdirs:
	cd $(top_srcdir) && $(MAKE) -f admin/Makefile.common subdirs

$(top_srcdir)/acinclude.m4: $(top_srcdir)/admin/acinclude.m4.in $(top_srcdir)/admin/libtool.m4.in
	@cd $(top_srcdir) && cat admin/acinclude.m4.in admin/libtool.m4.in > acinclude.m4

MAINTAINERCLEANFILES = subdirs configure.in acinclude.m4 configure.files 

dist-hook:
	cd $(top_distdir) && perl admin/am_edit -padmin
	cd $(top_distdir) && $(MAKE) -f admin/Makefile.common subdirs

EXTRA_DIST = examples/w3.bmp examples/qtapp/main.cpp examples/qtapp/myqt.cpp examples/qtapp/myqt.h examples/qtapp/qtapp.pro \
examples/qtgl/main.cpp  examples/qtgl/myqgl.cpp  examples/qtgl/myqgl.h  examples/qtgl/qtgl.pro ksquirrellibs.pc.in required-etch \
description-pak admin/acinclude.m4.in  admin/ChangeLog       admin/config.guess  admin/config.sub            admin/cvs-clean.pl  admin/depcomp             admin/Doxyfile.global  admin/ltmain.sh        admin/mkinstalldirs \
admin/am_edit          admin/compile         admin/config.pl     admin/configure.in.bot.end  admin/cvs.sh        admin/detect-autoconf.sh  admin/install-sh       admin/Makefile.common  admin/nmcheck \
admin/bcheck.pl        admin/conf.change.pl  admin/config.rpath  admin/configure.in.min      admin/debianrules   admin/Doxyfile.am         admin/libtool.m4.in    admin/missing          admin/ylwrap