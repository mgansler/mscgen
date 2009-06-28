#!/bin/bash
#
# $Id$
#

for F in `ls *.msc` ; do 
	echo $F
	./parsertest < $F > /dev/null || exit $?
done

# END OF SCRIPT
