# Build normally for Linux/Unix

export MSCGEN_VER=0.16

all:
	$(MAKE) -C src/parser
	$(MAKE) -C src/renderer

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

install:
	if [ -e ../bin/mscgen.exe ] ; then \
  		install -m 0755 bin/mscgen.exe /usr/bin; \
  		[ -r bin/bgd.dll ] && install -m 0755 bin/bgd.dll /usr/bin || true; \
	fi
	install -m 0755 bin/mscgen /usr/bin
	install -m 0644  man/mscgen.1 /usr/share/man/man1

uninstall:
	rm -f /usr/bin/mscgen /usr/bin/mscgen.exe /usr/bin/bgd.dll /usr/share/man/man1/mscgen.1

srcdist:
	$(MAKE) distclean
	(cd .. && tar -cv mscgen --exclude=.svn | gzip -9 - > mscgen-src-$(MSCGEN_VER).tar.gz)
	cp -f ../mscgen-src-$(MSCGEN_VER).tar.gz dist/mscgen-src-$(MSCGEN_VER).tar.gz
	openssl md5 dist/mscgen-src-$(MSCGEN_VER).tar.gz > dist/mscgen-src-$(MSCGEN_VER).tar.gz.md5

winbindist: all-cygwin
	rm -rf /tmp/mscgen
	mkdir /tmp/mscgen
	cp bin/bgd.dll /tmp/mscgen
	cp bin/mscgen.exe /tmp/mscgen
	cp man/mscgen.1 /tmp/mscgen
	(cd /tmp && zip -9r mscgen-w32-$(MSCGEN_VER).zip mscgen)
	mv -f /tmp/mscgen-w32-$(MSCGEN_VER).zip dist
	openssl md5 dist/mscgen-w32-$(MSCGEN_VER).zip > dist/mscgen-w32-$(MSCGEN_VER).zip.md5

bindist: all
	rm -rf /tmp/mscgen
	mkdir /tmp/mscgen
	cp bin/mscgen /tmp/mscgen
	cp man/mscgen.1 /tmp/mscgen
	(cd /tmp && tar -cv mscgen | gzip -9 - > mscgen$(STATIC)-$(MSCGEN_VER).tar.gz)
	mv -f /tmp/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz dist
	openssl md5 dist/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz > dist/mscgen$(STATIC)-$(MSCGEN_VER).tar.gz.md5

dllcheck:
	$(MAKE) -C src/renderer $@

distclean: clean
	$(MAKE) -C src/parser $@
	$(MAKE) -C src/renderer $@
	$()
	rm -f dist/*.gz dist/*.md5 dist/*.zip bin/mscgen.exe bin/mscgen bin/bgd.dll

clean:
	$(MAKE) -C src/parser $@
	$(MAKE) -C src/renderer $@

test:
	$(MAKE) -C src/parser $@
	$(MAKE) -C src/renderer $@

.PHONY: test bindist winbindist srcdist install uninstall clean distclean
