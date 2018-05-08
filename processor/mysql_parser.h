#ifndef _MYSQL_PARSER_H
#define _MYSQL_PARSER_H

#include "stream.h"

#define MYSQL_MAX_APP_PROTO_LENGTH 64
#define MYSQL_COMPRESS		0x20
#define MYSQL_PROTOCOL_41	0x200
#define MYSQL_SSL		0x800

#define MYSQL_QUIT		        0x01
#define MYSQL_INIT_DB	        0x02
#define MYSQL_QUERY		        0x03
#define MYSQL_SHOW_FIELD		0x04
#define MYSQL_CREATE_DB			0x05
#define MYSQL_DROP_DB			0x06
#define MYSQL_CONNECT			0x0b
#define MYSQL_STMT_PREPARE      0x16
#define MYSQL_STMT_EXECUTE		0x17
#define MYSQL_CHANGE_USER		0x11
#define MYSQL_RESPONSE_OK		0x00
#define MYSQL_RESPONSE_ERROR	0xFF
#define MYSQL_RESPONSE_EOF		0xFE

typedef struct _mysql_server_greeting {
	char protocol;
	char version[64];
	int thread_id;
	char salt1[8];
	unsigned short capabilities;
	char charset;
	unsigned short status;
	char reverve[13];
	char salt2[12];
} mysql_server_greeting;

typedef struct _msyql_logon_request {
    unsigned short capabilities;
    unsigned short ex_capabilities;
    unsigned int max_packet;
    char charset;
    char resvered[23];
    char username[128];
    short password_len;
    char password[128];
    char dbname[128];
    unsigned int other_payload_len;
    char other_payload[512];
} msyql_logon_request;

typedef struct _mysql_packet {
    int audit;
    int stream_dir;
    char *data;
    int data_len;
    
    stream_t *cli_s;
    stream_t *srv_s;
    
    unsigned short client_capablites;
    
    char sql[MAX_SQL_LEN];
	int sql_len;
	char prepare_sql[MAX_SQL_LEN];
	int prepare_sql_len;
	int last_res_type;
	int prepare_bind_var_num;
	int status;
	struct timeval begin_time;
	struct timeval end_time;
	int need_res;
	char error_message[512];
} mysql_packet;

struct mysql_thread_info {    /* Used as argument to thread_start() */
   pthread_t thread_id;        /* ID returned by pthread_create() */
   //mysql_packet *package;      /* From command-line argument */
   //session_t *session;
   int stream_dir;
   int f_socket;
   int t_socket;
   //char *buf;
   int bufsize;
};

#define mysql_out_uint8(p, v)\
{\
  *(p) = (u_int8_t)(v); \
  p++; \
}

#define mysql_out_uint16_le(p, v) \
{ \
  *(p) = (u_int8_t)((v) >> 0); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 8); \
  (p)++; \
}

#define mysql_out_uint24_le(p, v) \
{ \
  *(p) = (u_int8_t)((v) >> 0); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 8); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 16); \
  (p)++; \
}

#define mysql_out_uint32_le(p, v) \
{ \
  *(p) = (u_int8_t)((v) >> 0); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 8); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 16); \
  (p)++; \
  *(p) = (u_int8_t)((v) >> 24); \
  (p)++; \
}

#define mysql_out_uint8p(s, v, n) \
{ \
  memcpy((p), (v), (n)); \
  (p) += (n); \
}

#define mysql_out_uint8s(s, n) \
{ \
  memset((p), 0, (n)); \
  (p) += (n); \
}

#define mysql_out_string_null(p, v) \
{ \
  memcpy((p), (v), strlen(v)); \
  (p) += strlen(v); \
  *(p) = 0; \
  (p) += 1; \
}

#define mysql_in_uint8(p, v) \
{ \
  (v) = *((u_int8_t*)((p))); \
  (p)++; \
}

#define mysql_in_uint16_le(p, v) \
{ \
  (v) = (u_int16_t) \
    ( \
      (*((u_int8_t*)((p) + 0)) << 0) | \
      (*((u_int8_t*)((p) + 1)) << 8) \
    ); \
  (p) += 2; \
}

#define mysql_in_uint24_le(p, v) \
{ \
  (v) = (u_int32_t) \
    ( \
      (*((u_int8_t*)((p) + 0)) << 0) | \
      (*((u_int8_t*)((p) + 1)) << 8) | \
      (*((u_int8_t*)((p) + 2)) << 16) \
    ); \
  (p) += 3; \
}

#define mysql_in_uint32_le(p, v) \
{ \
  (v) = (u_int32_t) \
    ( \
      (*((u_int8_t*)((p) + 0)) << 0) | \
      (*((u_int8_t*)((p) + 1)) << 8) | \
      (*((u_int8_t*)((p) + 2)) << 16) | \
      (*((u_int8_t*)((p) + 3)) << 24) \
    ); \
  (p) += 4; \
}

#define mysql_in_string_null(p, v) \
{ \
   while(*(p))\
   {\
      *(v) = *((u_int8_t*)((p))); \
      (p)++; \
      (v)++; \
   }\
   (p)++;\
}

#define mysql_in_uint8p(p, v, n) \
{ \
  memcpy((v), (p), (n)); \
  (p) += (n); \
}

mysql_packet* mysql_decode_init();
int mysql_decode(session_t *session, mysql_packet *packet);
int mysql_decode_fee(mysql_packet *packet);
#endif
