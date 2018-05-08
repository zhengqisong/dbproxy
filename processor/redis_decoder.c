#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
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
#include <string.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "etcp.h"
#include "misc.h"
#include "sys_conf.h"
#include "proto_parser_conf.h"
#include "redis_parser.h"
#include "g_string.h"
#include "session.h"

redis_packet* redis_decode_init()
{
    redis_packet *p;
    int buf_len = sizeof(redis_packet);
    p = malloc(buf_len);
    
    memset(p, 0, buf_len);
    
    p->req_command.data = malloc(REDIS_COMMAND_DATA_SIZE);
    p->req_command.size = REDIS_COMMAND_DATA_SIZE;
    p->req_command.buf = p->req_command.data;
    p->req_command.multibulklen = 0;
    p->req_command.bulklen = -100;
        
    p->res_command.data = malloc(REDIS_COMMAND_DATA_SIZE);
    p->res_command.size = REDIS_COMMAND_DATA_SIZE;
    p->res_command.buf = p->res_command.data;
    p->res_command.multibulklen = 0;
    p->res_command.bulklen = -100;
    
    p->proc_buf = malloc(REDIS_PROTO_PROC_BUF_SIZE);
    p->proc_buf_size = REDIS_PROTO_PROC_BUF_SIZE;
    
    if(!p->req_command.data || !p->res_command.data || !p->proc_buf){
        dbgmsg("redis_decode_init error 3");
		goto err;
	}
    
	
	return p;
err:

	return NULL;
	
}

int redis_decode_fee(redis_packet *packet)
{
    if(packet)
    {
        if(packet->req_command.data)
            free(packet->req_command.data);
        if(packet->res_command.data)
            free(packet->res_command.data);
        if(packet->proc_buf)
            free(packet->proc_buf);
        free(packet);
    }
    return 0;
}

int redis_decode_addReplyDisplay(int sock, char *message, int len){
    int res;   
    res = tcp_send(sock, message, len);
    return res;
}

int redis_decode_addReplyError(int sock, char *message, int len){
    int res;
    tcp_send(sock, "-ERR ", 5);    
    tcp_send(sock, message, len);
    res = tcp_send(sock, "\r\n",2);
    return res;
}

int redis_decode_addReplyStatus (int sock, char *message, int len){
    int res;
    
    tcp_send(sock, "+", 5);    
    tcp_send(sock, message, len);
    res = tcp_send(sock, "\r\n",2);
    return res;
}

int redis_decode_addReplyInteger(int sock, int len){
    char buf[64];
    int res;
    
    sprintf(buf, ":%d\r\n", len);
    
    res = tcp_send(sock, buf, strlen(buf));
    return res;
}

int redis_decode_addReplyParam(int sock, char *message, int len){
    char buf[64];
    int res;
    
    sprintf(buf, "$%d\r\n", len);
    
    tcp_send(sock, buf, strlen(buf));    
    tcp_send(sock, message, len);
    res = tcp_send(sock, "\r\n",2);
    return res;
}

int redis_decode_addReplyMultiBulk (int sock, int len){
    char buf[64];
    int res;
    
    sprintf(buf, "*%d\r\n", len);
    
    res = tcp_send(sock, buf, strlen(buf));
    return res;
}

int redis_decode_find_crlf(char *str, int len)
{
    char *p = str + 1;
    char *end = str + len;
    char *pre;
    
    for( ; p < end; p++)
    {
        pre = (p-1);
        if(pre[0] == '\r' && p[0] == '\n')
        {
            return p - str + 1;
        }
    }
    return 0;
}

int redis_decode_splitargs(redis_packet *packet, int *argc) {
    char *p = packet->req_command.data;    
    char *begin = NULL, *end = NULL;
    char *current = packet->proc_buf;
    *argc = 0;
    while(1) {
        while(*p && isspace(*p)) p++;
        if (*p) {
            /* get a token */
            int inq=0;  /* set to 1 if we are in "quotes" */
            int insq=0; /* set to 1 if we are in 'single quotes' */
            int done=0;
            int escape = 0;
            begin = p;
            end = p;
            while(!done) {
                if (inq) {
                    if (*p == '\\') {
                        escape++;
                        escape = escape%2;
                    } else if(*p == '"' && !escape) {
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                        end = p;
                    } else {
                        escape = 0;
                    }
                } else if(insq) {
                    if (*p == '\\') {
                        escape++;
                        escape = escape%2;
                    } else if(*p == '\'' && !escape) {
                        if (*(p+1) && !isspace(*(p+1))) goto err;
                        done=1;
                        end = p;
                    } else {
                        escape = 0;
                    }
                } else {
                    switch(*p) {
                    case ' ':
                    case '\n':
                    case '\r':
                    case '\t':
                    case '\0':
                        done=1;
                        end = p;
                        break;
                    case '"':
                        inq=1;
                        begin++;
                        break;
                    case '\'':
                        insq=1;
                        begin++;
                        break;
                    default:
                        //current = sdscatlen(current,p,1);
                        break;
                    }
                }
                if (*p) p++;
            }
            (*argc)++;
            
            int argv_len = end - begin;
            int pos = current - packet->proc_buf;
            int need_len = pos + argv_len;
            
            //printf("need_len=%d, pos=%d,argv_len=%d, buf_size=%d,argc=%d\n", need_len, pos, argv_len, packet->proc_buf_size, *argc);
            if(need_len >= packet->proc_buf_size){
                packet->proc_buf_size = packet->proc_buf_size *((need_len % packet->proc_buf_size) + 1);
                
                if(realloc(packet->proc_buf, packet->proc_buf_size) == NULL) goto err;
                current = packet->proc_buf + pos;
            }
            //printf("current-proc_buf=%d,%s\n", current-packet->proc_buf, begin);
            memcpy(current, begin, argv_len);
            current += argv_len;
            current[0] = '\0';
            current++;
        }else{
            return 0;
        }
    }    
err:
    return -1;    
}

int redis_decode_parser_request_multibulk(session_t *session, redis_packet *packet)
{
    //return 0:正常转发,1:drop
    int res;
    char *newline = NULL;
    char *p = packet->req_command.data;
    int data_len = packet->req_command.data_len;
    int pos = 0, skip = 0, len = 0;
    char password[128]= {'\0'};
    
    if (packet->req_command.multibulklen == 0) {
        gettimeofday(&packet->begin_time, NULL);
        packet->req_command.argc = 0;
        memset(packet->req_command.command, 0, sizeof(packet->req_command.command));
        packet->sql_len = 0;
        
        //读multibulklen
        newline = strchr(p,'\r');
        if (newline == NULL) {
            if (data_len > REDIS_PROTO_INLINE_MAX_SIZE) {
                redis_decode_addReplyError(packet->fd->c_sock,"Protocol error: too big inline request", strlen("Protocol error: too big inline request"));
                return REDIS_C_ERR;
            }
        }
        /* Buffer should also contain \n */
        if (newline - p > data_len - 2)
            return REDIS_C_ERR;
        
        long param_num = atol(p+1);
        pos = (newline - p) + 2;
        
        if(param_num < 0)
        {
            return REDIS_C_ERR;
        }
        
        packet->req_command.multibulklen = param_num;
    }
    
    //packet->proc_buf
    while(packet->req_command.multibulklen) {
        /* Read bulk length if unknown */
        if (packet->req_command.bulklen == -100) {
            newline = strchr(p + pos,'\r');
            
            //check buf len
            if (newline == NULL) {
                if (data_len > REDIS_PROTO_INLINE_MAX_SIZE) {
                    redis_decode_addReplyError(packet->fd->c_sock,"Protocol error: too big inline request", strlen("Protocol error: too big inline request"));
                    return REDIS_C_ERR;
                }
                break;
            }
            
            /* Buffer should also contain \n */
            if (newline - p > data_len - 2)
                return REDIS_C_ERR;
            
            if (p[pos] != '$') {
                redis_decode_addReplyError(packet->fd->c_sock,"Protocol error: expected '$'", strlen("Protocol error: expected '$'"));
                return REDIS_C_ERR;
            }
            
            long param_len = atol(p + pos + 1);
            //new pos
            pos += newline-(p + pos)+2;
            
            packet->req_command.bulklen = param_len;
        }
        
        if(packet->req_command.argc == 0) {
            /* Read bulk argument */
            if(data_len - pos < packet->req_command.bulklen + 2) {
                pos = 0;
                packet->req_command.multibulklen = 0;
                packet->req_command.bulklen = -100;
                break;
            }
            //audit copy            
            memcpy(packet->sql, p + pos, packet->req_command.bulklen);
            packet->sql_len = packet->req_command.bulklen;
            memcpy(packet->sql + packet->sql_len, " " , 1);
            packet->sql_len += 1;
            
            /* Copy command */
            memcpy(packet->req_command.command, p + pos, packet->req_command.bulklen);
            pos += packet->req_command.bulklen + 2;
            packet->req_command.bulklen = -100;
            packet->req_command.multibulklen--;
            packet->req_command.argc++;
            
            //在这里可以获得用户输入的命令，可以处理将某个命令禁止执行，被禁的命令可以设置skip=1, 并在multibulklen=0时，向客户端输出提示信息;            
        } else {
            if(data_len - pos >= packet->req_command.bulklen + 2)
            {
                if(strncasecmp(packet->req_command.command, "auth", 4) == 0){
                    //获取密码
                    memcpy(password, p + pos, packet->req_command.bulklen);
                    password[packet->req_command.bulklen] = '\0';
                } else {  
                    //audit sql 
                    len = MAX_SQL_LEN - packet->sql_len - 1;
                    len = len > packet->req_command.bulklen ? packet->req_command.bulklen : len;
                    memcpy(packet->sql + packet->sql_len, p + pos, len); 
                    packet->sql_len += len;
                    if(packet->req_command.multibulklen == 1){
                        packet->sql[packet->sql_len] = '\0';
                    }else{
                        memcpy(packet->sql + packet->sql_len, " ", 1);
                        packet->sql_len += 1;
                    }
                }
                
                pos += packet->req_command.bulklen + 2;
                packet->req_command.bulklen = -100;
                packet->req_command.multibulklen--;
                packet->req_command.argc++;
            } else {
                //audit sql
                len = MAX_SQL_LEN - packet->sql_len - 1;
                len = len > (data_len - pos) ? (data_len - pos) : len;
                memcpy(packet->sql + packet->sql_len, p + pos, len); 
                packet->sql_len += len;
                
                //sub bulklen, add pos
                packet->req_command.bulklen = packet->req_command.bulklen - (data_len - pos);
                pos = data_len;
            }
        }
    }
    
    //if(packet->req_command.multibulklen == 0){
    //    printf("audit sql: %s\n", packet->sql);
    //}
    
    //change auth package
    if(strncasecmp(packet->req_command.command, "auth", 4) == 0) {
        if(packet->req_command.multibulklen != 0) {
            packet->req_command.argc = 0;
            packet->req_command.multibulklen = 0;
            packet->req_command.bulklen = -100;        
            pos = 0;
        } else {
            //获取密码
            if(strncmp(password, packet->auth_password, strlen(packet->auth_password))) {
                redis_decode_addReplyError(packet->fd->c_sock, "invalid password", strlen("invalid password"));
            } else {
                //send auth to server
                redis_decode_addReplyMultiBulk(packet->fd->s_sock, 2);
                redis_decode_addReplyParam(packet->fd->s_sock, "auth", 4);
                redis_decode_addReplyParam(packet->fd->s_sock, packet->server_password, strlen(packet->server_password));
            }
            skip = 1;
        }
    }
    
    //send data to server
    if(pos > 0){
        if(!skip){
            res = tcp_send(packet->fd->s_sock, packet->req_command.data, pos);
            if(res != 0 ) {
                return REDIS_C_ERR;
            }
        }
        
        if(pos == data_len) {
            packet->req_command.data_len = 0;
            packet->req_command.buf = packet->req_command.data;
        } else {
            memcpy(packet->req_command.data, packet->req_command.data+pos, data_len - pos);            
            packet->req_command.data_len = data_len - pos;
            packet->req_command.buf = packet->req_command.data + packet->req_command.data_len;
        }
    }    
    return 0;    
}

int redis_decode_parser_request_inline(session_t *session, redis_packet *packet)
{
    int res;
    char *newline;
    char *p = packet->req_command.data;
    int data_len = packet->req_command.data_len;
    int next_data_len = 0;
    
    newline = strchr(p,'\n');
    if (newline == NULL) {
        if (data_len > REDIS_PROTO_INLINE_MAX_SIZE) {
            redis_decode_addReplyError(packet->fd->c_sock,"Protocol error: too big inline request", strlen("Protocol error: too big inline request"));
            return REDIS_C_ERR;
        }
        
        /* Check req_command data the remaining space */
        if(packet->req_command.size < REDIS_PROTO_INLINE_MAX_SIZE 
            && (packet->req_command.size - packet->req_command.data_len) < REDIS_COMMAND_DATA_SIZE/3)
        {
            packet->req_command.size = packet->req_command.size + REDIS_COMMAND_DATA_SIZE;
            if(realloc(packet->req_command.data, packet->req_command.size)==NULL) return REDIS_C_ERR;
        }
        packet->req_command.buf = packet->req_command.data + data_len;
        
        return REDIS_C_OK;
    }
    
    /* Handle the \r\n case. */
    if (newline && newline != p && *(newline-1) == '\r')
        newline--;
    
    //分隔获取命令及参数
    int argc=0;
    res = redis_decode_splitargs(packet, &argc);
    
    if(res != 0 ) goto ret;
    
    gettimeofday(&packet->begin_time, NULL);
    
    //发送数据
    //判断是否为认证命令
    char *argv = packet->proc_buf;
    if(argc > 1 && strcasecmp(argv, "auth")==0){
        argv = argv + strlen(packet->proc_buf) + 1;
        if(argc != 2) {
            redis_decode_addReplyError(packet->fd->c_sock, "wrong number of arguments for 'auth' command", strlen("wrong number of arguments for 'auth' command"));
        } else if(strncmp(argv, packet->auth_password, strlen(packet->auth_password))) {
           redis_decode_addReplyError(packet->fd->c_sock, "invalid password", strlen("invalid password"));
        } else {            
            //send auth to server
            redis_decode_addReplyMultiBulk(packet->fd->s_sock, 2);
            redis_decode_addReplyParam(packet->fd->s_sock, "auth", 4);
            redis_decode_addReplyParam(packet->fd->s_sock, packet->server_password, strlen(packet->server_password));
            packet->req_command.buf = packet->req_command.data;
        }
    } else {
        //发送数据
        res = tcp_send(packet->fd->s_sock, packet->req_command.data, data_len);
        
        //保存用户执行的sql
        memcpy(packet->sql, packet->req_command.data, data_len > MAX_SQL_LEN ? MAX_SQL_LEN : data_len);
        packet->sql_len = data_len > MAX_SQL_LEN ? MAX_SQL_LEN : data_len;
    }
    
    //搬移剩余的数据
    next_data_len = (packet->req_command.data + packet->req_command.data_len) - (newline + 2);
    if(next_data_len != 0)
    {
        memcpy(packet->req_command.data, newline + 2, next_data_len);
        packet->req_command.data_len = next_data_len;
        packet->req_command.buf = packet->req_command.data + next_data_len;
    } else {
        packet->req_command.data_len = 0;
        packet->req_command.buf = packet->req_command.data;
    }
    
ret:
    return res;
}

int redis_decode_parser_request(session_t *session, redis_packet *packet)
{
    int res = 0;
    char c;
    char *p = packet->req_command.data;
    c = p[0];
    switch(c)
    {
        case '*':
            redis_decode_parser_request_multibulk(session, packet);
            break;
        default:
            res = redis_decode_parser_request_inline(session, packet);
            break;
        
    }
    
    //printf("datalen:%d,data:%s", data_len, packet->req_command.data);
//    res = tcp_send(packet->fd->s_sock, packet->req_command.data, data_len);
//    packet->req_command.data_len = 0;
    //printf("send to server res:%d", res);
    return res;
}

int redis_decode_parser_response(session_t *session, redis_packet *packet)
{
    char *p = packet->res_command.data;
    int data_len = packet->res_command.data_len;
    int res = 0;
    char *pr = NULL;
    char *result = NULL;
    if(packet->sql_len > 0){
        float e_time = 0;
		gettimeofday(&packet->end_time, NULL);
		
	    e_time = (packet->end_time.tv_sec - packet->begin_time.tv_sec) + (packet->end_time.tv_usec-packet->begin_time.tv_usec)/1000/1000.0;
	    if(e_time < 0)
	    {
	      e_time = 0;   
	    }
		    
        if(p[0] == '-') {
            result = "failure";
            strncpy(packet->error_message, p + 1, sizeof(packet->error_message));
            pr = strchr(packet->error_message, '\r');
            if(pr){
                pr[0]='\0';
            }
        } else {
            result = "success";
        }
        send_audit_message(session, result, e_time, packet->sql, packet->error_message);
        packet->sql_len = 0;
    }
    //printf("datalen:%d,data:%s", data_len, packet->res_command.data);
    res = tcp_send(packet->fd->c_sock, packet->res_command.data, data_len);
    packet->res_command.data_len = 0;
    //printf("send to client res:%d", res);
    
    return res;
}

int redis_decode(session_t *session, redis_packet *packet)
{    
    if(packet->stream_dir == DIR_REQUEST)
    {
        return redis_decode_parser_request(session, packet);
    }
    else
    {
        return redis_decode_parser_response(session, packet);
    }
}
