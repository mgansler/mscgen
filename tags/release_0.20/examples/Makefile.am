#
# $Id$
#


.SUFFIXES= .msc .mscin

example_DATA = msg_types.msc \
                boxes_example.msc \
                colour_sample.msc \
                simple_prog_desc.msc \
                client_server.msc

exampledir=$(docdir)/examples

# Use the suffix rule to support non-GNU make.
.mscin.msc:
	echo "#!$(bindir)/mscgen -Tpng" | cat - $< > $@
	chmod a+x $@

BUILT_SOURCES = $(example_DATA)

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = msg_types.mscin \
             boxes_example.mscin \
             colour_sample.mscin \
             simple_prog_desc.mscin \
             client_server.mscin

