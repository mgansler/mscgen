#!/bin/bash
#
# $Id$
#

libtoolize
aclocal
autoconf
autoheader
automake --add-missing

# END OF SCRIPT
