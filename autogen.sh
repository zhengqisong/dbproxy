#!/bin/sh
#autoscan

aclocal
autoconf
automake --add-missing
./configure CPPFLAGS="-DHEART_BEAT -O2 -Wno-pointer-sign -fno-strict-aliasing"
