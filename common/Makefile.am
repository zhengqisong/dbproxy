lib_LIBRARIES=libcommon.a
#bin_PROGRAMS=libcommon

SUBDIRS=util socket trace conf

libcommon_a_SOURCES=

libcommon_a_LIBADD=util/*.o trace/*.o socket/*.o conf/*.o

LDFLAGS=-s

AM_CFLAGS = -Wall -Werror

CFLAGS=-g

#EXTRA_DIST = Readme.txt
