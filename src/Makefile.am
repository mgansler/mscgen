#
# $Id$
#

AM_CFLAGS = -Wall -std=gnu99
AM_YFLAGS = -d

BUILT_SOURCES = language.c language.h lexer.c

CLEANFILES = $(BUILT_SOURCES)

# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = mscgen
mscgen_SOURCES = \
adraw.c      cmdparse.c  main.c      svg_out.c     language.y \
adraw.h      cmdparse.h  msc.c       ps_out.c      utf8.c \
adraw_int.h  gd_out.c    msc.h       safe.c        utf8.h \
bool.h       lexer.l     null_out.c  safe.h


if USE_WIN32_GD

# Add the required extra flags for the packaged Win32 binary (libbgd)
AM_CPPFLAGS=-I"$(srcdir)/../gdwin32/include/"
mscgen_LDADD =-L"$(srcdir)/../gdwin32/lib/" -L"$(srcdir)/../gdwin32/bin/" -lbgd

# Add the DLL as a script
#  This is a bit of a hack, but puts the DLL in the same directory as the exe
#  and with execute permissions set.
bin_SCRIPTS = $(srcdir)/../gdwin32/bin/bgd.dll

else
mscgen_LDADD = -lgd
endif

# END OF FILE