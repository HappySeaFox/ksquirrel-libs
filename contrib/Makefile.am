SUBDIRS = 

if SQ_HAVE_SVG
SUBDIRS += libpixman cairo libsvg libsvg-cairo
endif

if SQ_HAVE_GIF
SUBDIRS += libungif
endif

if SQ_HAVE_JPEG2000
SUBDIRS += libjasper
endif

if SQ_HAVE_WMF
SUBDIRS += libwmf
endif

if SQ_HAVE_OPENEXR
SUBDIRS += libopenexr
endif

install: 