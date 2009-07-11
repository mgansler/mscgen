AUTOMAKE_OPTIONS=subdir-objects

## Default CFLAGS/CPPFLAGS/LDFLAGS besides user-provided ones.
## Automake automatically adds -I some args (e.g. -I.).
## These can be set on a "per [something]" basis.
AM_CFLAGS   = -Wall -Wextra
## config.h is in src/ and we are in src/renderer/
AM_CPPFLAGS = -DHAVE_CONFIG_H -I..

## Headers to install
nobase_include_HEADERS =

## Internal header files that should not be installed.
noinst_HEADERS  = adraw.h adraw_int.h utf8.h cmdparse.h

## Add mscgen to the list of programs to be made and installed in $prefix/bin
bin_PROGRAMS    = mscgen
## List the source-files required to build mscgen.
mscgen_SOURCES  = adraw.c cmdparse.c gd_out.c null_out.c svg_out.c utf8.c main.c
mscgen_SOURCES += ps_out.c


mscgen_CPPFLAGS=
mscgen_LDFLAGS =

if USE_WIN32_GD
## Add the required extra flags...
mscgen_CPPFLAGS+=-I"$(srcdir)/../../gdwin32/include/"
mscgen_LDFLAGS +=-L"$(srcdir)/../../gdwin32/lib/" -L"$(srcdir)/../../gdwin32/bin/"  -lbgd
else
## Specify linker flags.
## NB: automake figures the linker flags to handle libparser.la
mscgen_LDFLAGS  += -lgd
endif


## Use library created by us previously.
mscgen_LDADD     = ../parser/libparser.la

## Add the -I to get the headers from ../parser/
## Use the $(srcdir) since we need the "source-folder" and not the
## build folder.
##
## NB: headers from yacc/bison may be in the build folder!
mscgen_CPPFLAGS += $(AM_CPPFLAGS) -I$(srcdir)/../parser/ -I../parser

