MAKE=gmake -w
INSTALL_DIR=/usr/lib/squirrel

all: clean
	$(MAKE) -C bmp
	$(MAKE) -C jpeg
	$(MAKE) -C png

install:
	@echo "Remove old libraries from $(INSTALL_DIR) ? (y/n) "
	@read answer
	@if [ "${answer}" = "y" ]; then rm -rf $(INSTALL_DIR); else echo; fi
	@echo "Create $(INSTALL_DIR) ? (y/n) "
	@read answer
	@if [ "${answer}" = "y" ]; then mkdir $(INSTALL_DIR); else echo; fi
	$(MAKE) install -C bmp
	$(MAKE) install -C jpeg
	$(MAKE) install -C png
	@echo "Copying developing headers into /usr/include ..."
	@if test -n "/usr/include/squirrel/"; then echo; else mkdir /usr/include/squirrel; fi
	@cp defs.h /usr/include/squirrel/
	@cp err.h /usr/include/squirrel/
	@cp endian.h /usr/include/squirrel/

clean:
	$(MAKE) clean -C bmp
	$(MAKE) clean -C jpeg
	$(MAKE) clean -C png
