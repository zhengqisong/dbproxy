#ifndef _REDIS_PARSER_H
#define _REDIS_PARSER_H

#include "stream.h"
#define REDIS_C_OK                    0
#define REDIS_C_SKIP                  -1
#define REDIS_C_ERR                   -2

#define REDIS_PROTO_REQ_INLINE 1
#define REDIS_PROTO_REQ_MULTIBULK 2

#define REDIS_COMMAND_DATA_SIZE 1024
#define REDIS_PROTO_INLINE_MAX_SIZE   (1024*64) /* Max size of inline reads */

#define REDIS_PROTO_PROC_BUF_SIZE 512

typedef struct _redis_command{
    char *data;
    int size;
    char *buf;
    int data_len;
    int type;
    char command[32];
    int pos;
    int multibulklen;
    int bulklen;
    int argc;
} redis_command;

typedef struct _redis_packet {
    int audit;
    char auth_password[36];
    char server_password[36];
    int stream_dir;
    
    redis_command req_command;
    redis_command res_command;
    socket_fd *fd;
    
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
	
	char *proc_buf;
	int proc_buf_size;
} redis_packet;

redis_packet* redis_decode_init();
int redis_decode(session_t *session, redis_packet *packet);
int redis_decode_fee(redis_packet *packet);

#endif
