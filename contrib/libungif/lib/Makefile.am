## Process this file with automake to produce a Makefile.in for libungif.

EXTRA_DIST = Makefile.unx

lib_LTLIBRARIES = libungif.la

include_HEADERS = gif_lib.h

libungif_la_SOURCES = dgif_lib.c  \
                    gif_err.c   \
                    gif_font.c  \
                    gifalloc.c  \
                    qprintf.c   \
                    gif_lib_private.h

## Version is 4.1.3  [x:y:z  (x - z).(y).(z) ]
libungif_la_LDFLAGS = -version-info 5:3:1
libungif_la_LIBADD = @DEVS@
AM_CFLAGS = @CFLAGS@ $(X_CFLAGS) -Wall
