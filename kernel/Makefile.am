SUBDIRS = jpeg png bmp gif pix xwd xpm xbm tiff tga sgi ras pnm pcx ico xcur psd fli iff utah cut wmf rawrgb wbmp sun camera

EXTRA_DIST = fmt_defs.h error.h fio.h fmt_codec_base.h fmt_utils.h fmt_types.h generate

install-data-local:
	$(mkinstalldirs) $(DESTDIR)/usr/include/ksquirrel-libs
	$(INSTALL_DATA)  fmt_defs.h          $(DESTDIR)/usr/include/ksquirrel-libs/fmt_defs.h
	$(INSTALL_DATA)  fmt_types.h         $(DESTDIR)/usr/include/ksquirrel-libs/fmt_types.h
	$(INSTALL_DATA)  fmt_utils.h         $(DESTDIR)/usr/include/ksquirrel-libs/fmt_utils.h
	$(INSTALL_DATA)  fmt_codec_base.h    $(DESTDIR)/usr/include/ksquirrel-libs/fmt_codec_base.h
	$(INSTALL_DATA)  error.h             $(DESTDIR)/usr/include/ksquirrel-libs/error.h
	$(INSTALL_DATA)  fio.h               $(DESTDIR)/usr/include/ksquirrel-libs/fio.h