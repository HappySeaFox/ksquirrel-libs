install-data-local:
	$(mkinstalldirs) $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources
	$(mkinstalldirs) $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c
	$(mkinstalldirs) $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++
	$(mkinstalldirs) $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx
	
	$(INSTALL_DATA)   c/module.c                $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c/module.c
	$(INSTALL_DATA)   c/main.c                  $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c/main.c
	$(INSTALL_SCRIPT) c/compile                 $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c/compile
	
	$(INSTALL_SCRIPT)  c++/compile              $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++/compile
	$(INSTALL_DATA)    c++/main.cpp             $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++/main.cpp
	$(INSTALL_DATA)    c++/triangle.cpp         $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++/triangle.cpp
	$(INSTALL_DATA)    c++/polygon.hpp          $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++/polygon.hpp
	$(INSTALL_DATA)    c++/README               $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/c++/README
	
	$(INSTALL_DATA)    ttx/Makefile.am          $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/Makefile.am
	$(INSTALL_DATA)    ttx/README               $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/README
	$(INSTALL_SCRIPT)  ttx/compile-c++          $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/compile-c++
	$(INSTALL_DATA)    ttx/fmt_codec_ttx_defs.h $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/fmt_codec_ttx_defs.h
	$(INSTALL_DATA)    ttx/fmt_codec_ttx.h      $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/fmt_codec_ttx.h
	$(INSTALL_DATA)    ttx/fmt_codec_ttx.cpp    $(DESTDIR)/usr/share/doc/ksquirrel-libs/$(PACKAGE_VERSION)/sources/ttx/fmt_codec_ttx.cpp

EXTRA_DIST = c c++ ttx