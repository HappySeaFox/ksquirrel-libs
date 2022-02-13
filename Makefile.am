SUBDIRS = kernel doc

AUTOMAKE_OPTIONS = foreign 

pkgconfigdir = ${exec_prefix}/lib/pkgconfig
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
examples/qtgl/main.cpp  examples/qtgl/myqgl.cpp  examples/qtgl/myqgl.h  examples/qtgl/qtgl.pro ksquirrellibs.pc.in