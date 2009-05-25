# Build normally for Linux/Unix

export MSCGEN_VER=0.14

all:
	(cd parser && $(MAKE))
	(cd renderer && $(MAKE) && $(MAKE) dllcheck)

all-osx:
	(export OS=osx; $(MAKE))

all-freebsd:
	(export OS=freebsd; \
	 export CFLAGS="-I /usr/local/include $(CFLAGS)"; \
	 export LDFLAGS="-L /usr/local/lib $(LDFLAGS)"; \
	 $(MAKE))

# Build for cygwin, or win32-mingw.
#  Both are the same and produce a native Win32 binary.
all-cygwin all-w32-mingw:
	(export CFLAGS="-mno-cygwin $(CFLAGS)";\
	 export LDFLAGS="-mno-cygwin $(LDFLAGS)"; \
	 export OS=win32; \
	 $(MAKE))

srcdist:
	$(MAKE) distclean
	(cd ../.. && tar -cv mscgen --exclude=.svn | gzip -9 - > mscgen-src-$(MSCGEN_VER).tar.gz)
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
	$(MAKE) -C test $@
	$(MAKE) -C parser $@
	$(MAKE) -C renderer $@	
	$()
	rm -f ../dist/*.gz ../dist/*.md5 ../dist/*.zip ../bin/mscgen.exe ../bin/mscgen ../bin/bgd.dll

clean:
	$(MAKE) -C test $@
	$(MAKE) -C parser $@
	$(MAKE) -C renderer $@	

%:
	(cd parser && $(MAKE) $@)
	(cd renderer && $(MAKE) $@)

test:
	(cd test && ./parsercheck.sh)
	(cd test && ./renderercheck.sh)
	
.PHONY: test bindist winbindist srcdist clean distclean
