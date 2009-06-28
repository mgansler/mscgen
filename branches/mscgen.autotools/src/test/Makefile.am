AUTOMAKE_OPTIONS=subdir-objects

TESTS = parsercheck.sh renderercheck.sh
check_PROGRAMS = parsertest

parsertest_SOURCES = ./parsertest.c
parsertest_LDADD = ../parser/libparser.la
parsertest_CPPFLAGS= -I$(srcdir)/../parser/

clean-local:
	-rm -f *.png *.eps

EXTRA_DIST =  parsercheck.sh renderercheck.sh

EXTRA_DIST+= 	\
		testinput0.msc \
		testinput1.msc \
		testinput2.msc \
		testinput3.msc \
		testinput4.msc \
		testinput5.msc \
		testinput6.msc \
		testinput7.msc \
		testinput8.msc \
		testinput9.msc