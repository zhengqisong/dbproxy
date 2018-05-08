noinst_LIBRARIES=libutil.a

libutil_a_SOURCES=misc.c g_sysv.c g_string.c g_number.c g_list.c g_xml.c g_expect.c g_io.c g_getinfo.c g_getethinfo.c MiscDecoder.c g_timer.c my_sha1.c g_uuid.c g_cjson.c g_regex.c  

INCLUDES=-I../trace -I../conf -I/usr/local/include/libxml2

AM_CFLAGS=-Wall -Wno-strict-aliasing -finline-functions -rdynamic

LIBS=-lpthread -L/usr/lib -ldl -lm -lutil -L/usr/local/lib -lxml2 -luuid

CFLAGS=-g

#EXTRA_DIST = Readme.txt
