noinst_LIBRARIES=libprocessor.a

SUBDIRS=

libprocessor_a_SOURCES=proto_parser_conf.c mysql_proc.c mysql_decoder.c mysql_cache.c redis_cache.c redis_decoder.c redis_proc.c

INCLUDES=-I/usr/local/include/libxml2 -I../common/conf -I../common/trace -I../common/socket -I../common/util -I../responsor

AM_CFLAGS=-Wall -finline-functions -rdynamic

LIBS=-lpthread -L/usr/lib -ldl -lm -L/usr/local/lib -lxml2 -lcrypto -lssl
CFLAGS=-g

#EXTRA_DIST = Readme.txt