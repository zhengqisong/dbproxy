#!/bin/sh
#autoscan
aclocal
autoconf
automake --add-missing
./configure CPPFLAGS="-Wno-pointer-sign"


