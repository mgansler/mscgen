#
# $Id$
#

%.msc: %.msc.in
	echo "#!$(bindir)/mscgen -Tpng" | cat - $< > $@


BUILT_SOURCES = msg_types.msc \
                boxes_example.msc \
                colour_sample.msc \
                simple_prog_desc.msc

CLEANFILES = $(BUILT_SOURCES)

EXTRA_DIST = msg_types.msc.in \
             boxes_example.msc.in \
             colour_sample.msc.in \
             simple_prog_desc.msc.in

