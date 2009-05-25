#!/bin/bash

for F in `ls *.msc` ; do
	echo "$F"
	../../bin/mscgen -T png -o $F.png -i $F || exit $?
done

# END OF SCRIPT
