#
# $Id$
#

AC_INIT([mscgen],[0.17],[Michael.McTernan.2001@cs.bris.ac.uk])
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

AC_ARG_WITH([png],
  [AS_HELP_STRING([--without-png],
    [Remove png-support and dependence on libgd @<:@default=no@:>@])],
  [if test "$withval" = "no" ; then
     AC_DEFINE_UNQUOTED(REMOVE_PNG_OUTPUT, 1)
   else
     PKG_CHECK_MODULES(GDLIB, gdlib)
   fi],
  [PKG_CHECK_MODULES(GDLIB, gdlib)]
)

AH_TEMPLATE([REMOVE_PNG_OUTPUT],
            [If set, remove PNG output support thereby removing libgd dependence.])

# Older versions of autoconf (<2.60?) fail to define docdir 
docdir='${datadir}/doc/mscgen'
AC_SUBST(docdir)

# Earlier versions of gdlib.pc fail to include -lgd in the link flags.
#  Check if it needs to be added.  This allows building on RedHat EL
AC_SEARCH_LIBS(gdImageColorAllocate, gd,, AC_MSG_ERROR([Failed to find library containing gd]), -lm)


AC_OUTPUT

