#!/bin/sh

set -x
libtoolize --automake --copy --force
aclocal-1.11
autoconf --force
autoheader --force
automake-1.11 --add-missing --copy --force-missing --foreign
