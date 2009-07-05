# Makefile for msc parser library

YACC=bison
LEX=flex

LDFLAGS+=
CC=gcc
CFLAGS+=-O2 -I. -Wall -Wextra
OBJS  =language.tab.o lex.yy.o msc.o

libparser.a: $(OBJS)
	$(AR) cr $@ $^

language.tab.c language.tab.h: language.y
	$(YACC) -d $^

lex.yy.c: language.l language.tab.h
	$(LEX) -s $^

parsertest: parsertest.o libparser.a
	$(CC) $(LDFLAGS) -o $@ $^


test: parsertest
	cd ../test && ./parsercheck.sh

clean: FORCE
	rm -f libparser.a language.tab.c language.tab.h lex.yy.c parsertest.o language.output $(OBJS)

distclean: clean
	rm -f parsertest parsertest.exe

FORCE:
# DO NOT DELETE
