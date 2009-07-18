#
# $Id$
#
# Makefile for mscgen program

YACC=bison
LEX=flex
CC=gcc
CFLAGS+=-Wall -Wextra -I../parser -DPACKAGE_VERSION="\"$(MSCGEN_VER)\"" -O2
OBJS=main.o null_out.o gd_out.o ps_out.o svg_out.o cmdparse.o adraw.o utf8.o language.tab.o lex.yy.o msc.o safe.o

ifeq ($(OS),win32)

GDWIN32=../gdwin32
CFLAGS+=-I$(GDWIN32)/include


# On win32, use the binary package that is a part of the mscgen
#  distribution.  This makes the probability if the build succeeding
#  much higher in a tricky environment.
../bin/mscgen: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(GDWIN32)/lib/bgd.lib
	cp $(GDWIN32)/bin/bgd.dll ../bin
	chmod a+x ../bin/bgd.dll

done=1

endif

ifeq ($(OS),osx)

# Minor changes from the POSIX build for OSX to remove -static and add
# the macports libpath
../bin/mscgen: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ -L/opt/local/lib -lgd -lpng -lz -lfreetype -lm

done=1

endif

ifndef done

# Ideally we could use pkg-config here:
#   LDFLAGS+=`pkg-config $(STATIC) --libs gdlib`
#
# However, this often misses -lgd, may return -ldg, and includes more
# libraries than actually required for mscgen (which then creates extra
# dependencies).  Instead the libs can be listed manually for a static
# link:

LDFLAGS+=-lgd

ifeq ($(STATIC),-static)
LDFLAGS+=-fontconfig -lpng12 -lz -lm
endif

# Linux, assume that the gd-devel libs are already installed and link
#  against them.  Note that version 2.0.22 is needed for the new
#  functions gdFontGetTiny(), gdFontGetSmall() etc...
../bin/mscgen: $(OBJS)
	$(CC) -o $@ $^ $(STATIC) $(LDFLAGS)

endif

# Main image

language.tab.c language.tab.h: language.y
	$(YACC) -d $^

lex.yy.c: language.l language.tab.h
	$(LEX) -s $^

distclean: clean
	(cd ../bin && rm -f mscgen.exe mscgen bgd.dll)

clean:
	rm -f $(OBJS)
	rm -f language.tab.c language.tab.h lex.yy.c parsertest.o safe.o language.output
	rm -f ../test/*.png ../test/*.eps ../test/*.svg ../test/*.ismap

ifeq ($(OS),win32)
dllcheck:
	-objdump -x ../bin/mscgen.exe | grep "DLL Name"
else
ifeq ($(OS),osx)
dllcheck:
	-otool -L ../bin/mscgen
else
dllcheck:
	-objdump -T ../bin/mscgen
endif
endif

parsertest: parsertest.o language.tab.o lex.yy.o msc.o safe.o
	$(CC) $(LDFLAGS) -o $@ $^

test: parsertest
	(cd ../test && ./parsercheck.sh)
	(cd ../test && ./renderercheck.sh)

.PHONY: clean distclean test dllcheck
