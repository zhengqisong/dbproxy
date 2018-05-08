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
#include <string.h>
#include <sys/time.h>
#include <openssl/md5.h>
#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "etcp.h"
#include "misc.h"
#include "sys_conf.h"
#include "my_global.h"
#include "my_sha1.h"
#include "proto_parser_conf.h"
#include "mysql_parser.h"
#include "g_string.h"
#include "session.h"

mysql_packet* mysql_decode_init()
{
    mysql_packet *p;
    int buf_len = sizeof(mysql_packet);
    p = malloc(buf_len);
    
    memset(p, 0, buf_len);
    
    //cli_s
	make_stream(p->cli_s);
	if(!p->cli_s)
	{
		dbgmsg("mysql_decode_init error 1");
		goto err;
	}
	init_stream(p->cli_s, 2 * 1024);
	if(!p->cli_s->data)
	{
		dbgmsg("mysql_decode_init error 2");
		goto err;
	}

	//srv-s
	make_stream(p->srv_s);
	if(!p->srv_s)
	{
		dbgmsg("mysql_decode_init error 3");
		goto err;
	}
	init_stream(p->srv_s, 2 * 1024);
	if(!p->srv_s->data)
	{
		dbgmsg("mysql_decode_init error 4");
		goto err;
	}
	
	
	return p;
err:

	return NULL;
	
}

int mysql_decode_fee(mysql_packet *packet)
{
    if(packet)
    {
        free_stream(packet->cli_s);
        free_stream(packet->srv_s);
        free(packet);
    }
    return 0;
}

int get_encode_by_charset(unsigned short charset, char *encode)
{
	int ret = 0;
	if(charset == 1 || charset == 84)
	{
		memcpy(encode, "big5", strlen("big5"));
	}
	else if(charset == 28 || charset == 87)
	{
		memcpy(encode, "gbk", strlen("gbk"));
	}
	else if(charset == 8 || charset == 15 || charset == 31 || charset == 47 || charset == 48 || charset == 49 || charset == 94)
	{
		memcpy(encode, "latin1", strlen("latin1"));
	}
	else if(charset == 24)
	{
		memcpy(encode, "gb2312", strlen("gb2312"));
	}
	else if(charset == 33 || charset == 88 || charset > 192)
	{
		memcpy(encode, "utf-8", strlen("utf-8"));
	}
	else
	{
		ret = -1;
	}
	return ret;
}

int in_lenenc_int(char *begin, char *end, int64_t *num)
{
	unsigned char c;
	int len = 0;

	c = *begin;
	if(c < 0xfb)
	{
		*num = c;
		len = 1;
	}
	else if(c == 0xfc)
	{
		if(begin + 3 >= end)
			goto end;
		*num = (int64_t)(
				(((int64_t)(u_int8_t)begin[1]) << 0)
				|(((int64_t)(u_int8_t)begin[2]) << 8)
				);
		len = 3;
	}
	else if(c == 0xfd)
	{
		if(begin + 4 >= end)
			goto end;
		*num = (int64_t)(
				(((int64_t)(u_int8_t)begin[1]) << 0)
				|(((int64_t)(u_int8_t)begin[2]) << 8)
				|(((int64_t)(u_int8_t)begin[3]) << 16)
				);
		len = 4;
	}
	else if(c == 0xfe)
	{
		if(begin + 9 >= end)
			goto end;
		*num = (int64_t)(
						(((int64_t)(u_int8_t)begin[1]) << 0)
						|(((int64_t)(u_int8_t)begin[2]) << 8)
						|(((int64_t)(u_int8_t)begin[3]) << 16)
						|(((int64_t)(u_int8_t)begin[4]) << 24)
						|(((int64_t)(u_int8_t)begin[5]) << 32)
						|(((int64_t)(u_int8_t)begin[6]) << 40)
						|(((int64_t)(u_int8_t)begin[7]) << 48)
						|(((int64_t)(u_int8_t)begin[8]) << 56)
						);
		len = 9;
	}

end:
	return len;
}

int mysql_decode_request_type(session_t *session, mysql_packet *packet, char* begin, char* end, char* type, int len)
{
	int ret = 0;
	int sql_len = end - begin;

	if(sql_len > MAX_SQL_LEN - 1 - len)
	{
		sql_len = MAX_SQL_LEN - 1 - len;
	}

	if(len > 0)
	{
		memcpy(packet->sql, type, len);
	}
	memcpy(packet->sql + len, begin, sql_len);

	packet->sql[sql_len + len] = 0;
	packet->sql_len = sql_len + len;
	return ret;
}

int mysql_decode_request_execute(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	//需要解析参数, 查看 COM_STMT_PREPARE response

	memcpy(packet->sql, packet->prepare_sql, packet->prepare_sql_len);
	packet->sql_len = packet->prepare_sql_len;
	return ret;
}

int mysql_decode_request_prepare(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	int sql_len = end - begin;

	if(sql_len > MAX_SQL_LEN - 1)
	{
		sql_len = MAX_SQL_LEN - 1;
	}
	memcpy(packet->prepare_sql, begin, sql_len);
	packet->prepare_sql[sql_len] = 0;
	packet->prepare_sql_len = sql_len;
	return ret;
}


int mysql_decode_request(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	char *data = begin;
	int packet_len = 0;
	int sequence_id = 0 ;
    int len = 0;
    
	unsigned char type = 0;

	in_uint24_le(data, packet_len);
	sequence_id = *(data);
	data++;
	
	gettimeofday(&packet->begin_time, NULL);
	packet->need_res = 0;
	
	if(sequence_id == 1)
	{
		//登录包，目前发现只有登录包的和超大包的序号为一的可能，而超大包，目前不考虑
		//ret = mysql_decode_requst_login(packet, mpd, data, end);
	}
	else
	{
		type = *(data);
		dbgmsg("request type=%d",type);
		
		
		switch(type)
		{
			case MYSQL_QUIT:
				break;
			case MYSQL_INIT_DB:
				ret = mysql_decode_request_type(session, packet, data + 1, end, "use ", strlen("use "));
				if(packet->sql_len>0){
				    len = packet->sql_len-4 > sizeof(session->dbname) ? sizeof(session->dbname)-1 : packet->sql_len-4;
				    memcpy(session->dbname, packet->sql + strlen("use "), len);
				    session->dbname[len] = '\0';
				}
				break;
			case MYSQL_QUERY:
			    packet->need_res = 1;
				ret = mysql_decode_request_type(session, packet, data + 1, end, "", 0);
				break;
			case MYSQL_SHOW_FIELD:
				break;
			case MYSQL_CREATE_DB:
				ret = mysql_decode_request_type(session, packet, data + 1, end, "create database ", strlen("create database "));
				break;
			case MYSQL_DROP_DB:
				ret = mysql_decode_request_type(session, packet, data + 1, end, "drop database ", strlen("drop database "));
				break;
			case MYSQL_CONNECT:
				break;
			case MYSQL_STMT_PREPARE:
				ret = mysql_decode_request_prepare(session, packet, data + 1, end);
				break;
			case MYSQL_STMT_EXECUTE:
			    packet->need_res = 1;
				ret = mysql_decode_request_execute(session, packet, data + 1, end);
				break;
			case MYSQL_CHANGE_USER:
				ret = mysql_decode_request_type(session, packet, data + 1, end, "change ", strlen("change "));
				break;
			default:
				 break;
		}
	}
	return ret;
}

int mysql_decode_response_ok(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	char *data = begin;
	int len = 0;
	
	int64_t effect_row = 0;
	int64_t last_insert_id = 0;

	len = in_lenenc_int(data, end, &effect_row);
	data += len;
	len = in_lenenc_int(data, end, &last_insert_id);
	data += len;
	
	if(last_insert_id > 0){
	    sprintf(packet->error_message, "effect_row=%d,last_insert_id=%d", (int)effect_row, (int)last_insert_id);
    } else {
        sprintf(packet->error_message, "effect_row=%d", (int)effect_row);
    }
	return ret;
}

int mysql_decode_response_error(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	char *data = begin;
	int len = 0;
	int error_code = 0;

	in_uint16_le(data, error_code);
	if(packet->client_capablites & MYSQL_PROTOCOL_41)
	{
		data += 5;
	}

	len = end - data;
	if(len >= 512)
	{
		len = 510;
	}
	memcpy(packet->error_message, data, len);
	return ret;
}

int mysql_decode_response_eof(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
//	char *data = begin;
//	int len = 0;

	return ret;
}

int mysql_decode_response(session_t *session, mysql_packet *packet, char* begin, char* end)
{
	int ret = 0;
	char *data = begin;
	int packet_len = 0;
	int sequence_id = 0 ;

	unsigned char type = 0;

	in_uint24_le(data, packet_len);
	sequence_id = *(data);
	data++;
    
    gettimeofday(&packet->end_time, NULL);
	packet->need_res = 0;
	
	type = *(data);
	switch(type)
	{
		case MYSQL_RESPONSE_OK:
			ret = mysql_decode_response_ok(session, packet, data + 1, end);
			break;
		case MYSQL_RESPONSE_ERROR:
			ret = mysql_decode_response_error(session, packet, data + 1, end);
			break;
		case MYSQL_RESPONSE_EOF:
			ret = mysql_decode_response_eof(session, packet, data + 1, end);
			break;
		default:
		    //返回值，type表示返回结果的列数
		    break;
	}
	packet->last_res_type = type;
	
	return ret;
}

int mysql_decode_data(session_t *session, mysql_packet *packet, char* begin, char* end)
{
    int ret = 0;
	if(packet->stream_dir == DIR_REQUEST)
	{
		ret = mysql_decode_request(session, packet, begin, end);
	}
	else
	{
		ret = mysql_decode_response(session, packet, begin, end);
	}

	return ret;
}

int mysql_decode(session_t *session, mysql_packet *packet)
{
    int buf_len;
    stream_t* s;
    char *data;
	int data_len;
	unsigned int len;
	
	char* begin;
	char* end;
	char* next_packet;
	
	data = packet->data;
	data_len = packet->data_len;
	
    s = (DIR_REQUEST == packet->stream_dir ? packet->cli_s : packet->srv_s);
    
    //拷贝新接收的数据到stream_t中
    buf_len = s->end - s->data;
	buf_len += data_len;
	
    resize_stream(s, buf_len);
    memcpy(s->end, data, data_len);
	s->end += data_len;

	s->p = s->data;

	buf_len = s->end - s->data;
    
    //分析数据包
    begin = s->p;
	end = s->end;
    while(buf_len > 0)
	{
		//判断是否有一个完成的数据包
		if((s->end - s->p) < 4)
		{
			goto end;
		}
		
		//包长度，不含通用包头
		stream_in_uint24_le(s, len);
		len += 4;
		if((s->end - s->data) < len)
		{
			goto end;
		}
		
		s->p -= 3;
		//下一个数据的位置
		next_packet = s->data + len;
		//当前包的结束位置
		s->end = next_packet;
		
		//清空sql
		if(packet->need_res==0)
		{
    		packet->sql[0] = 0;
    		packet->sql_len = 0;
	    }
	    memset(packet->error_message, 0, sizeof(packet->error_message));
	    
		mysql_decode_data(session, packet, s->p, s->end);
		
		//将未解析的数据包移到最前面
		memcpy(s->data, next_packet, buf_len - len);
		s->p = s->data;
		s->end = s->data + (buf_len - len);
		buf_len -= len;
		
		if(packet->sql_len && packet->need_res==0)
		{
		    float e_time = 0;
		    char *result = NULL;
		    e_time = (packet->end_time.tv_sec - packet->begin_time.tv_sec) + (packet->end_time.tv_usec-packet->begin_time.tv_usec)/1000/1000.0;
		    if(e_time < 0)
		    {
		      e_time = 0;   
		    }
		    
		    if(packet->last_res_type == MYSQL_RESPONSE_ERROR){
		        result = "failure";
		    }else{
		        result = "success";   
		    }
		    send_audit_message(session, result, e_time, packet->sql, packet->error_message);
		}
	}
end:
    return 0;
}
