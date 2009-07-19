#
# $Id$
#


.SUFFIXES= .msc .mscin
exampledir=$(docdir)/examples

# Use the suffix rule to support non-GNU make.
.mscin.msc:
	echo "#!$(bindir)/mscgen -Tpng" | cat - $< > $@
	chmod a+x $@

BUILT_SOURCES = msg_types.msc \
                boxes_example.msc \
                colour_sample.msc \
                simple_prog_desc.msc

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = msg_types.mscin \
             boxes_example.mscin \
             colour_sample.mscin \
             simple_prog_desc.mscin


if WITH_EXAMPLES
install-examples: $(BUILT_SOURCES)
	test -d $(DESTDIR)$(exampledir)/ || $(MKDIR_P) $(DESTDIR)$(exampledir)/
	$(INSTALL_SCRIPT) *.msc $(DESTDIR)$(exampledir)/

uninstall-examples:
	rm -f $(DESTDIR)$(exampledir)/*.msc
else
install-examples:
uninstall-examples:

endif


install-data-hook: install-examples
uninstall-hook: uninstall-examples
