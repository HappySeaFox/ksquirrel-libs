AC_INIT([ksquirrel-libs], [0.5.0-pre4], [ksquirrel@tut.by])
AC_GNU_SOURCE
#AC_CONFIG_SRCDIR([config.h.in])
#AM_INIT_AUTOMAKE([foreign])
#AC_CONFIG_HEADER([config.h])

AC_CONFIG_AUX_DIR([admin])

AC_CANONICAL_SYSTEM
AC_ARG_PROGRAM

AM_INIT_AUTOMAKE(ksquirrel-libs, 0.5.0-pre4)

AC_CHECK_COMPILERS
AC_ENABLE_SHARED(yes)
AC_ENABLE_STATIC(no)
AC_PROG_LIBTOOL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([limits.h stdlib.h strings.h unistd.h stdio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset], [], AC_MSG_ERROR("memset couldn'tbe found"))
AC_CHECK_FUNCS([memcpy], [], AC_MSG_ERROR("memcpy couldn'tbe found"))
AC_CHECK_FUNCS([setjmp], [], AC_MSG_ERROR("setjmp couldn'tbe found"))

AC_CHECK_HEADER([sys/wait.h], [AC_DEFINE(_SYS_WAIT_H)], [AC_MSG_ERROR([Sorry, can't do anything for you])])

sq_have_jpeg="yes"
sq_have_png="yes"
sq_have_xwd="yes"
sq_have_tiff="yes"

AC_CHECK_LIB([jpeg], [main], [], [sq_have_jpeg="no"])
AC_CHECK_LIB([png], [png_create_read_struct], [], [sq_have_png="no"])

AC_CHECK_HEADERS([jpeglib.h jconfig.h jerror.h jmorecfg.h], [], [sq_have_jpeg="no"])
AC_CHECK_HEADERS([png.h], [], [sq_have_png="no"])
AC_CHECK_HEADERS([X11/XWDFile.h], [], [sq_have_xwd="no"])
AC_CHECK_HEADERS([tiffio.h tiff.h], [], [sq_have_tiff="no"])

AC_CONFIG_SUBDIRS(contrib/libungif)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([contrib/Makefile])
AC_CONFIG_FILES([kernel/Makefile])
AC_CONFIG_FILES([kernel/bmp/Makefile])
AC_CONFIG_FILES([kernel/gif/Makefile])
AC_CONFIG_FILES([kernel/pix/Makefile])
AC_CONFIG_FILES([kernel/xbm/Makefile])
AC_CONFIG_FILES([kernel/xpm/Makefile])
AC_CONFIG_FILES([kernel/tga/Makefile])
AC_CONFIG_FILES([kernel/sgi/Makefile])
AC_CONFIG_FILES([kernel/ras/Makefile])
AC_CONFIG_FILES([kernel/pnm/Makefile])
AC_CONFIG_FILES([kernel/pcx/Makefile])
AC_CONFIG_FILES([kernel/ico/Makefile])

if test $sq_have_png = yes; then
AC_CONFIG_FILES([kernel/png/Makefile])
fi

if test $sq_have_jpeg = yes; then
AC_CONFIG_FILES([kernel/jpeg/Makefile])
fi

if test $sq_have_xwd = yes; then
AC_CONFIG_FILES([kernel/xwd/Makefile])
fi

if test $sq_have_tiff = yes; then
AC_CONFIG_FILES([kernel/tiff/Makefile])
fi

AC_OUTPUT

echo "
KSquirrel-libs configuration:
-----------------------------
  Source code location: ${srcdir}
  KSquirrel version:    ${VERSION}
  C Compiler:           ${CC}
  C Compiler flags:     ${CFLAGS}
  C++ Compiler:         ${CXX}
  C++ Compiler flags:   ${CXXFLAGS}
  
Installation options:
---------------------
  libdir:               ${libdir}
  
Libraries:
----------"

sq_enabled="bmp gif pix xbm xpm tga sgi ras pnm pcx ico"
sq_disabled=""

if test $sq_have_jpeg = yes; then sq_enabled=`echo -n $sq_enabled jpeg `; else sq_disabled=`echo -n $sq_disabled jpeg `; fi
if test $sq_have_png = yes; then sq_enabled=`echo -n $sq_enabled png `; else sq_disabled=`echo -n $sq_disabled png `; fi
if test $sq_have_xwd = yes; then sq_enabled=`echo -n $sq_enabled xwd `; else sq_disabled=`echo -n $sq_disabled xwd `; fi
if test $sq_have_tiff = yes; then sq_enabled=`echo -n $sq_enabled tiff `; else sq_disabled=`echo -n $sq_disabled tiff `; fi

echo "Enabled:  $sq_enabled"
echo "Disabled: $sq_disabled"
echo
echo "Now run 'make' to compile libraries"
echo
