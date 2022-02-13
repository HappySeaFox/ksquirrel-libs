MAKE=gmake -w
INSTALL_DIR=/usr/lib/squirrel/

all: clean
	@mkdir $(INSTALL_DIR)
	$(MAKE) -C bmp
	$(MAKE) -C ico
	$(MAKE) -C jpg
	$(MAKE) -C pcx
	$(MAKE) -C pix
	$(MAKE) -C png
	$(MAKE) -C pnm
	$(MAKE) -C ras
	$(MAKE) -C sgi
	$(MAKE) -C tga
	$(MAKE) -C tiff
	$(MAKE) -C xbm
	$(MAKE) -C xpm
	$(MAKE) -C xwd

install:
	$(MAKE) install -C bmp
	$(MAKE) install -C ico
	$(MAKE) install -C jpg
	$(MAKE) install -C pcx
	$(MAKE) install -C pix
	$(MAKE) install -C png
	$(MAKE) install -C pnm
	$(MAKE) install -C ras
	$(MAKE) install -C sgi
	$(MAKE) install -C tga
	$(MAKE) install -C tiff
	$(MAKE) install -C xbm
	$(MAKE) install -C xpm
	$(MAKE) install -C xwd

clean:
	@echo "Remove old libraries from $(INSTALL_DIR) ? (y/n) "
	@read answer
	@if [ "${answer}" = "y" ]; then rm -rf $(INSTALL_DIR); else echo; fi
	$(MAKE) clean -C bmp
	$(MAKE) clean -C ico
	$(MAKE) clean -C jpg
	$(MAKE) clean -C pcx
	$(MAKE) clean -C pix
	$(MAKE) clean -C png
	$(MAKE) clean -C pnm
	$(MAKE) clean -C ras
	$(MAKE) clean -C sgi
	$(MAKE) clean -C tga
	$(MAKE) clean -C tiff
	$(MAKE) clean -C xbm
	$(MAKE) clean -C xpm
	$(MAKE) clean -C xwd
