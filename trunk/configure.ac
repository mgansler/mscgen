#
# $Id$
#

AC_INIT([mscgen],[0.18],[Michael.McTernan.2001@cs.bris.ac.uk])
AC_PREREQ(2.59)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([man/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
AC_PROG_CC
AM_PROG_CC_C_O
AM_PROG_LEX
AC_PROG_YACC
AC_PROG_INSTALL

AC_CHECK_HEADERS([unistd.h])
AC_CHECK_HEADERS([limits.h])

#
# Check if libgd is needed
#
AC_ARG_WITH([png],
  [AS_HELP_STRING([--without-png],
                  [Remove png-support and dependence on libgd @<:@default=no@:>@])],
  [if test "$withval" = "no" ; then
     AC_DEFINE_UNQUOTED(REMOVE_PNG_OUTPUT, 1)
     with_libgd=no
   fi]
)

AH_TEMPLATE([REMOVE_PNG_OUTPUT],
            [If set, remove PNG output support thereby removing libgd dependence.])


# Try to figure out how gd works if we need it.
#  GD may be packaged with either gdlib-config or using pkg-config.
#  This is a real pain, so we try both and hope one works okay.
#
if test "x$with_libgd" != "xno"; then


  # Get compile and link options from either gdlib-config or pkg-config
  AC_PATH_PROG(GDLIB_CONFIG,gdlib-config)
  if test -n "$GDLIB_CONFIG"; then
  
    # Allow command line override as per PKG_CHECK macro
    if test -z "$GDLIB_CFLAGS"; then
      GDLIB_CFLAGS="`$GDLIB_CONFIG --cflags`"
    fi
    if test -z "$GDLIB_LIBS"; then
      GDLIB_LIBS="`$GDLIB_CONFIG --ldflags` `$GDLIB_CONFIG --libs`"
    fi
    
  else
    PKG_CHECK_MODULES(GDLIB, gdlib)
  fi
  
  # Earlier versions of gdlib.pc fail to include -lgd in the link flags.
  #  Check if it needs to be added.  This allows building on RedHat EL
  AC_SEARCH_LIBS(gdImageColorAllocate, gd,, AC_MSG_ERROR([Failed to find library containing gd]), -lm)

  # Check if freetype is needed with gd
  AC_ARG_WITH([freetype],
    [AS_HELP_STRING([--with-freetype],
      [Add FreeType font rendering @<:@default=no@:>@])],
    [if test "$withval" = "yes" ; then
       AC_DEFINE_UNQUOTED(USE_FREETYPE, 1)
       PKG_CHECK_MODULES(FREETYPE, freetype2)
       AC_SEARCH_LIBS(gdImageStringFT, gd,, AC_MSG_ERROR([Failed to find library containing gdImageStringFt]))
     fi]
  )

fi


AH_TEMPLATE([USE_FREETYPE],
            [Use FreeType for rendering text in PNGs.])


# Older versions of autoconf (<2.60?) fail to define docdir
docdir='${datadir}/doc/mscgen'
AC_SUBST(docdir)


AC_OUTPUT

