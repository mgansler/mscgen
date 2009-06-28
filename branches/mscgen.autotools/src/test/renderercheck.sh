#!/bin/bash
#
# $Id$
#

for F in `ls *.msc` ; do
	echo "$F"
	../renderer/mscgen -T png -o $F.png -i $F || exit $?
done

# END OF SCRIPT
