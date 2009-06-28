AUTOMAKE_OPTIONS=


## Default CFLAGS/CPPFLAGS/LDFLAGS besides user-provided ones.
## Automake automatically adds -I some args (e.g. -I.).
## These can be set on a "per [something]" basis.
AM_CFLAGS = -Wall -Wextra
AM_CPPFLAGS=

## Default flags for yacc/bison and lex/flex respectively.
AM_YFLAGS = -d
AM_LFLAGS =

## Add libparser to the list of libraries to be built.
lib_LTLIBRARIES = libparser.la

## Public headers that should go in $(prefix)/include/[name]
## e.g. parser/bool.h will be installed as $(prefix)/include/bool.h
include_HEADERS=

## Public headers that should go in $(prefix)/include/[path/][name]
## e.g. parser/bool.h will be installed as $(prefix)/include/parser/bool.h
nobase_include_HEADERS=

## Private/Internal headers that should not be installed.
noinst_HEADERS=

## Perhaps consider to move them into a sub-folder src/mscgen/
## refer to them as mscgen/[header] in the sources +here and
## use the variable "nobase_include_HEADER" instead
## To reduce name clash.
include_HEADERS += msc.h bool.h

#noinst_LTLIBRARIES = libtmp.la
#libtmp_la_SOURCES = language.y 
#libtmp_la_LDFLAGS =  -release $(VERSION) -no-undefined

## The source-files required to build libparser.{so,a,la}
libparser_la_SOURCES = parser.y scanner.l msc.c

## Linker flags requried. The "-release [version] -no-undefined" are for libtool
## Setting the "version" of the library and the -no-undefined is required to crossbuild
## dll-files.
libparser_la_LDFLAGS = -release $(VERSION) -no-undefined

BUILT_SOURCES = parser.c parser.h scanner.c scanner.h

## Does not handle lang.c too well in distcheck
## so just remov them.
DISTCLEANFILES = language.h language.c