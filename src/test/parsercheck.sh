#!/bin/bash

for F in `ls *.msc` ; do 
	echo $F
	../parser/parsertest < $F > /dev/null || exit $?
done

# END OF SCRIPT
