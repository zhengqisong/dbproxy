noinst_LIBRARIES=libresponsor.a

SUBDIRS=

libresponsor_a_SOURCES=session.c

INCLUDES=-I/usr/local/include/libxml2 -I../common/conf -I../common/trace -I../common/socket -I../common/util

AM_CFLAGS=-Wall -finline-functions -rdynamic

LIBS=-lpthread -L/usr/lib -ldl -lm -L/usr/local/lib -lxml2 -lcrypto -lssl
CFLAGS=-g

#EXTRA_DIST = Readme.txt