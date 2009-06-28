## Name of package, version of package and maintainer-/bug-email
AC_INIT([mscgen],[0.16],[Michael.McTernan.2001@cs.bris.ac.uk])


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

## Can check for requirements e.g. support for C99.
AC_PROG_CC     #Find a C-compiler.
AM_PROG_CC_C_O #Required to build files across dirs.
AM_PROG_LEX    #Find a lexer (lex/flex/...)
AC_PROG_YACC   #Find a parser generator (yacc/bison/...)
AC_LIBTOOL_WIN32_DLL 
AC_PROG_LIBTOOL #Check for libtool - used to create libraries.

## Check for headers/functions here.
AC_CHECK_HEADERS([unistd.h])

## Define functions to try building and linking against gd.
##  This is done to catch the Cygwin case where -mno-cygwin may allow 
##  inclusion of gd.h, but no native lib exists.  The first function
##  simply trys to link with the installed gd, the second trys the 
##  packaged convenience lib.
AC_DEFUN([GD_TRY_LINK],
[dnl
	AC_MSG_CHECKING([whether $1 links ])

	_save_libs="$LIBS"
	LIBS="$LIBS $1"
	AC_LINK_IFELSE([#include "gd.h"
                        #include "gdfontt.h"  /* Tiny font */
                        int main(void) { 
                          return gdFontGetTiny() != NULL;
                        }
                        ],
                       [gd_flag=yes],
                       [gd_flag=no])
	LIBS="$_save_libs"

	if test "x$gd_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$gd_flag])
])


AC_DEFUN([GD_TRY_CONV_LINK],
[dnl
	AC_MSG_CHECKING([whether $1 links ])

	_save_libs="$LIBS"
	_save_ldflags="$LDFLAGS"
	_save_cppflags="$CPPFLAGS"
	CPPFLAGS="$CPPFLAGS -Igdwin32/include/"
	LDFLAGS="$LDFLAGS -Lgdwin32/lib/ -Lgdwin32/bin/"  
	LIBS="$LIBS $1"
	AC_LINK_IFELSE([#include "gd.h"
                        #include "gdfontt.h"  /* Tiny font */
                        int main(void) { 
                          return gdFontGetTiny() != NULL;
                        }
                        ],
                       [gd_flag=yes],
                       [gd_flag=no])
	LIBS="$_save_libs"
	LDFLAGS="$_save_ldflags"
	CPPFLAGS="$_save_cppflags"

	if test "x$gd_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$gd_flag])
])


GD_TRY_LINK([-lgd], [sh_HAVE_GD_H=1], [sh_HAVE_GD_H=0])

sh_USE_WIN32_GD=0

if test "$sh_HAVE_GD_H" = 0 ; then

   GD_TRY_CONV_LINK([-lbgd], [sh_USE_WIN32_GD=1], [AC_MSG_ERROR([mscgen requires libgd and gd.h; yum install libgd-devel?])])
   
fi

## This has to be run unconditionally.
## Otherwise automake will complain since USE_WIN32_GD will not always
## have a value.
AM_CONDITIONAL(USE_WIN32_GD, [test "$sh_USE_WIN32_GD" = 1])


AC_OUTPUT
