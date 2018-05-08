#ifndef _DECODE_PROTO_CONF_H
#define _DECODE_PROTO_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>
#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "etcp.h"
#include "misc.h"
#include "session.h"

#define PROTO_NUM	100

typedef struct _socket_fd {
	int c_sock;
	int s_sock;
} socket_fd;

typedef int (*ParserProc)(session_t *session, SOCKET cs, char* raw, int len);

typedef struct _parser_proto {
	int proto_id;
	char proto_name[64];
	ParserProc process;
} parser_proto;

extern parser_proto parser_protos[PROTO_NUM];

parser_proto* get_parser_proto_by_raw(char* protocol, char* raw, int len);

//通用 proc

//mysql
int mysql_proc(session_t *session, SOCKET cs, char* raw, int len);
//redis
int redis_proc(session_t *session, SOCKET cs, char* raw, int len);

#endif
