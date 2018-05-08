bin_PROGRAMS=proxy

proxy_SOURCES= proxy.c

INCLUDES=-I/usr/local/include/libxml2 -I../common/socket -I../common/util -I../common/trace -I../common/conf -I../processor  -I../responsor

LIBS=-lpthread -L/usr/local/ssl/lib -L/usr/lib -ldl -lm -L/usr/local/lib -lxml2 -lcrypto -lssl -luuid

AM_CFLAGS=-finline-functions -rdynamic -Wall -Werror

CFLAGS=-g -O2

proxy_LDADD=../processor/libprocessor.a ../responsor/libresponsor.a ../common/libcommon.a  

