# Makefile for msc parser library

YACC=bison
LEX=flex

LDFLAGS+=
CC=gcc
CFLAGS+=-O2 -I. -Wall
OBJS  =language.tab.o lex.yy.o msc.o

libparser.a: $(OBJS)
	$(AR) cr $@ $^

language.tab.c language.tab.h: language.y
	$(YACC) -d $^

lex.yy.c: language.l
	$(LEX) $^

parsertest: parsertest.o libparser.a
	$(CC) $(LDFLAGS) -o $@ $^


test: parsertest
	cat testinput0.msc | ./parsertest
	cat testinput1.msc | ./parsertest
	cat testinput2.msc | ./parsertest
	cat testinput3.msc | ./parsertest
	cat testinput4.msc | ./parsertest
	cat testinput5.msc | ./parsertest
	cat testinput6.msc | ./parsertest

clean: FORCE
	rm -f libparser.a language.tab.c lex.yy.c parsertest.o language.output $(OBJS)

distclean: clean
	rm -f parsertest parsertest.exe

FORCE:
# DO NOT DELETE
