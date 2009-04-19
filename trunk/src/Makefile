# Build normally for Linux/Unix

export MSCGEN_VER=0.13

all:
	(cd parser && $(MAKE))
	(cd renderer && $(MAKE) && $(MAKE) dllcheck)

all-osx:
	(export OS=osx; $(MAKE))

# Build for cygwin, or win32-mingw.
#  Both are the same and produce a native Win32 binary.
all-cygwin all-w32-mingw:
	(export CFLAGS="-mno-cygwin $(CFLAGS)";\
	 export LDFLAGS="-mno-cygwin $(LDFLAGS)"; \
	 export OS=win32; \
	 $(MAKE))

srcdist:
	$(MAKE) distclean
	(cd ../.. && tar -cv mscgen | gzip -9 - > mscgen-src-$(MSCGEN_VER).tar.gz)
	cp -f ../../mscgen-src-$(MSCGEN_VER).tar.gz ../dist/mscgen-src-$(MSCGEN_VER).tar.gz
	openssl md5 ../dist/mscgen-src-$(MSCGEN_VER).tar.gz > ../dist/mscgen-src-$(MSCGEN_VER).tar.gz.md5

winbindist: all-cygwin
	rm -rf /tmp/mscgen
	mkdir /tmp/mscgen
	cp ../bin/bgd.dll /tmp/mscgen
	cp ../bin/mscgen.exe /tmp/mscgen
	cp man/mscgen.1 /tmp/mscgen
	(cd /tmp && zip -9r mscgen-w32-$(MSCGEN_VER).zip mscgen)
	mv -f /tmp/mscgen-w32-$(MSCGEN_VER).zip ../dist
	openssl md5 ../dist/mscgen-w32-$(MSCGEN_VER).zip > ../dist/mscgen-w32-$(MSCGEN_VER).zip.md5

bindist: all
	rm -rf /tmp/mscgen
	mkdir /tmp/mscgen
	cp ../bin/mscgen /tmp/mscgen
	cp man/mscgen.1 /tmp/mscgen
	(cd /tmp && tar -cv mscgen | gzip -9 - > mscgen$(STATIC)-$(MSCGEN_VER).tar.gz)
	mv -f /tmp/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz ../dist
	openssl md5 ../dist/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz > ../dist/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz.md5

distclean: clean
	rm -f ../dist/*.gz ../dist/*.md5 ../dist/*.zip ../bin/mscgen.exe ../bin/mscgen ../bin/bgd.dll

clean:
	(cd parser && $(MAKE) $@)
	(cd renderer && $(MAKE) $@)
	rm -f test0.png test0.svg \
	test1.png test1.svg \
	test2.png test2.svg test2.ismap \
	test3.png test3.svg \
	test4.png test4.svg \
	test5.png test5.svg \
	test6.png test6.svg

%:
	(cd parser && $(MAKE) $@)
	(cd renderer && $(MAKE) $@)

test:
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput0.msc -o test0.png -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput0.msc -o test0.svg -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput1.msc -o test1.png -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput1.msc -o test1.svg -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput2.msc -o test2.png -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput2.msc -o test2.svg -p
	$(VALGRIND) ../bin/mscgen -T ismap -i parser/testinput2.msc -o test2.ismap -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput3.msc -o test3.png -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput3.msc -o test3.svg -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput4.msc -o test4.png -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput4.msc -o test4.svg -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput5.msc -o test5.svg -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput5.msc -o test5.png -p
	$(VALGRIND) ../bin/mscgen -T eps -i parser/testinput5.msc -o test5.eps -p
	$(VALGRIND) ../bin/mscgen -T svg -i parser/testinput6.msc -o test6.svg -p
	$(VALGRIND) ../bin/mscgen -T png -i parser/testinput6.msc -o test6.png -p


.PHONY: test bindist winbindist srcdist clean distclean
