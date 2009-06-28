## Name of package, version of package and maintainer-/bug-email
AC_INIT([mscgen],[0.15],[Michael.McTernan.2001@cs.bris.ac.uk])


AC_PREREQ(2.61)

AC_CONFIG_SRCDIR([src/])

## List "automake" makefiles.
AC_CONFIG_FILES([Makefile src/Makefile 
			  src/man/Makefile
			  src/parser/Makefile
			  src/renderer/Makefile
			  src/test/Makefile
			  ])

## List auto-created config file for  the build.
## source files should include this.
AC_CONFIG_HEADER([src/config.h])


## Setup for cross-building
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET


AM_INIT_AUTOMAKE

AC_PROG_INSTALL

## Expose maintainer-mode. Might come in handy.
AM_MAINTAINER_MODE


#Can check for requirements e.g. supprt for C99.
AC_PROG_CC     #Find a C-compiler.
AM_PROG_CC_C_O #Required to build files across dirs.
AM_PROG_LEX    #Find a lexer (lex/flex/...)
AC_PROG_YACC   #Find a parser generator (yacc/bison/...)
AC_LIBTOOL_WIN32_DLL 
AC_PROG_LIBTOOL #Check for libtool - used to create libraries.

## Check for headers/functions here.
AC_CHECK_HEADERS([unistd.h])

sh_HAVE_GD_H=0
AC_CHECK_HEADER([gd.h], [sh_HAVE_GD_H=1])

## With this we can use USE_WIN32_GB in Makefile.am to determine
## whether to add flags for the gdwin32

if test "$sh_HAVE_GD_H" = 0 -a "$EXEEXT" = ".exe" ; then
   AC_MSG_WARN([Using convenience gd library.])
elif test "$sh_HAVE_GD_H" = 0 ; then 
   AC_MSG_ERROR([mscgen requires libgd and gd.h])
fi
## This has to be run unconditionally.
## Else automake will complain since USE_WIN32_GD will not always
## have a value.
AM_CONDITIONAL(USE_WIN32_GD, [test "$sh_HAVE_GD_H" = 0 -a "$EXEEXT" = ".exe"])


AC_OUTPUT
