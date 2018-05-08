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
#include <time.h>
#include <openssl/md5.h>
#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "etcp.h"
#include "misc.h"
#include "sys_conf.h"
#include "proto_parser_conf.h"
#include "g_string.h"
#include "g_regex.h"

#include "session.h"

#include "redis_parser.h"
#include "redis_cache.h"

#define BUFSIZE	1024


/*----------------common---------------------------*/

int redis_get_errormsg(int err, char *buf)
{
    dbgmsg("error %d",err);
    switch(err)
    {
     case -2:
        strcpy(buf, "Interface error");
        break;
     case -3:
        strcpy(buf, "System error, please contact the administrator");
        break;
     case -4:
        strcpy(buf, "Invalid password");
        break;
     case -5:
        strcpy(buf, "Can not connect server");
        break;
     case -6:
        strcpy(buf, "server error");
        break;
     case -7:
        strcpy(buf, "The instance is locking");
        break;
     default:
        strcpy(buf, "Unknown authentication error");
        break;
    }
    return 0;
}

int redis_get_error_session_status(int err)
{
    dbgmsg("error %d",err);
    switch(err)
    {
     case -1:
        return SESSION_AUTH_ERR;
     case -2:
        return SESSION_INTERFACE_ERR;
     case -4:
        return SESSION_AUTH_ERR;
     case -5:
        return SESSION_TCP_ERR;
     case -7:
        return SESSION_LOCK_ERR;
     default:
        return SESSION_OTHER_ERR;
    }
}

/**
 * 读取通讯包头
 * 可用与读上行和下行数据包头
 */
int read_package(SOCKET sock, char *type, char *buffer, int buf_len)
{
    int res = 0;
    int param_num = 0; 
    res = readcrlf(sock, buffer, buf_len);
    if(res < 1){
        return -5;
    }
    
    if(buffer[0] == '*'){
         param_num = atoi(buffer+1);
         type[0] = '*';
    }
    else if(buffer[0] == '$'){
        param_num = atoi(buffer+1);
        type[0] = '$';
    }else if(buffer[0] == '+'){
        strcpy(buffer, buffer + 1);
        type[0] = '+';
        param_num = res - 1;
    }else{
        type[0] = '-';
        param_num = res;
    }
    return param_num;
}

/**
 * 发送命令并获取命令结果
 * 用于上行
 */
int redis_send_command_and_read_res(SOCKET sock, char *command, char *buffer, int len){
    int res;
    char type[1];
    
    //发送info replication
    //strcpy(buffer, "*2\r\n$4\r\ninfo\r\n$11\r\nreplication\r\n");
    if(tcp_send(sock, command, strlen(command)) == -1){
        goto error;
    }
    //read server data
    //$2323\r\ndata
    if((res = read_package(sock, type, buffer, len)) < 1){
       goto error;
    }
    if(type[0] != '$' && type[0] != '+'){
       goto error;
    }
    if(type[0] == '$'){
        //read data
        if(readn(sock, buffer, res + 2) == -1){
           goto error;
        }        
        buffer[res+1] = '\0';  
    }  
	return res;	
	
error:
    return -1;	 
}

/**
 * 获取客户端认证的密码信息
 */
int redis_client_auth(socket_fd *fd, char *password)
{
    char type[1];
    char buf[1024] = {'\0'};
    int res = 0;
    int param_num = 0;
    
    //#driver
    //0000   2a 32 0d 0a 24 34 0d 0a 41 55 54 48 0d 0a 24 38  *2..$4..AUTH..$8
    //0010   0d 0a 68 6f 75 68 74 31 32 33 0d 0a              ..houht123..
    //#telent
    //0000   41 55 54 48 20 68 6f 75 68 74 31 32 33 0d 0a     AUTH houht123
    res = read_package(fd->c_sock, type, buf, sizeof(buf));
    if(res < 1){
        return -4;
    }
    
    //driver
    if(type[0] == '*'){
        param_num = res;
        if(param_num != 2)
        {
           return -4;  
        }
        //read auth
        if((res = read_package(fd->c_sock, type, buf, sizeof(buf))) < 1){
           return -4;   
        }
        if(type[0] != '$')
        {
            return -4;
        }
        if((res = readcrlf(fd->c_sock, buf, sizeof(buf))) < 1){
           return -4;   
        }
        if(v_strncasecmp(buf, res, "AUTH", 4) != 0){
            return -4;
        }
        //read password
        if((res = read_package(fd->c_sock, type, buf, sizeof(buf))) < 1){
            return -4;
        }
        if(type[0] != '$')
        {
            return -4;
        }
        if((res = readcrlf(fd->c_sock, buf, sizeof(buf))) < 1)
        {
           return -4;   
        }
        strcpy(password, buf);
        return 0;
    }
    else if(type[0] == '-'){
        if(v_trim_str(buf) < 5){
            return -4;
        }
        
        if(v_strncasecmp(buf, res, "AUTH ", 5) != 0)
        {
            return -4;
        }
        if(v_trim_str(buf + 5) < 1)
        {
            return -4;
        }
        strcpy(password, buf + 5);
        return 0;
    }else{
        return -4;
    }
}

/**
 * 向服务端认证密码
 */
int redis_auth_use_password(SOCKET sock, char *password)
{
    char buffer[1024];
    
    int len = 0;
    int res;
    
    if(password!=NULL && strlen(password) > 0)
	{ 
	    len =strlen(password);
	    sprintf(buffer, "*2\r\n$4\r\nauth\r\n$%d\r\n%s\r\n", len, password);
	    if((res = redis_send_command_and_read_res(sock, buffer, buffer, strlen(buffer))) < 1){
                goto error;
        }
        if(v_strncasecmp(buffer, 2, "OK", 2) != 0){
            goto error;
        }
	}
	
	return 0;
	
error:
    return -1;	
	
}


/**
 * 连接服务端，如果认证密码不为空，将认证密码
 */
int redis_connect_node(char *host, char *port, char *password)
{
    SOCKET s_sock;
    
    dbgmsg("to connect db server");
    s_sock = tcp_client(host, port, 1, fatal_quit_f);
	if (s_sock < 0)
	{
		dbgmsg("connect server error server:(%s,%s)", host, port);
		return -1;
	}
	if(password != NULL && strlen(password) > 0 && redis_auth_use_password(s_sock, password) != 0)
	{
	    goto error;
	}
	return s_sock;
	
error:
    CLOSE_SOCKET(s_sock);
    s_sock = 0;
    return s_sock;
}

/**
 * 根据密码，查找用户实例，并主动连接master或slaver实例
 */
int redis_connect_instance(session_t *session, socket_fd *fd, char *password, redis_packet *packet)
{
    /* slave node
    #info replication
    $363
    # Replication
    role:slave
    master_host:10.0.2.80
    master_port:6379
    master_link_status:up
    master_last_io_seconds_ago:6
    master_sync_in_progress:0
    slave_repl_offset:255720
    slave_priority:100
    slave_read_only:1
    connected_slaves:0
    master_repl_offset:0
    repl_backlog_active:0
    repl_backlog_size:1048576
    repl_backlog_first_byte_offset:0
    repl_backlog_histlen:0
    */
    /* master node
    #info replication
    $252
    # Replication
    role:master
    connected_slaves:1
    slave0:ip=10.0.2.80,port=6380,state=online,offset=255706,lag=1
    master_repl_offset:255706
    repl_backlog_active:1
    repl_backlog_size:1048576
    repl_backlog_first_byte_offset:2
    repl_backlog_histlen:255705    
    */
    /*
    info sentinel
    # Sentinel
    sentinel_masters:1
    sentinel_tilt:0
    sentinel_running_scripts:0
    sentinel_scripts_queue_length:0
    sentinel_simulate_failure_flags:0
    master0:name=master-1,status=ok,address=10.0.2.80:6379,slaves=1,sentinels=1
    */
    /*
    0000   2b 4f 4b 0d 0a                                   +OK..
    */
    int res = -1;
    int i=0;
    int len =0;
    char buffer[1024];
    char masterhost[64] = {'\0'};
    char masterport[32] = {'\0'};
    
    
    //根据密码获取用户的redis实例
    redis_instance_info *instance_info = redis_cache_get_instance_info(password);    
    redis_node_info *node_info;
    
    if(instance_info == NULL)
    {
        fd->s_sock = -1;
        res = -4;
        goto ret;
    }
    //遍历实例节点，寻找合适的节点
    for(i = 0; i<instance_info->node_num; i++)
    {
        node_info = instance_info->nodes + i;
        strcpy(masterhost, node_info->host);
        sprintf(masterport,"%d", node_info->port);
        
        fd->s_sock = redis_connect_node(masterhost, masterport, NULL);
        if(fd->s_sock < 0)
        {
            goto next;
        }
        
        //通过sentinel节点，获取master
        len = strlen(node_info->sentinel);
        if(v_strncasecmp(node_info->sentinel, len, "yes", 3) == 0)
        {
            //发送info sentinel
            strcpy(buffer, "*2\r\n$4\r\ninfo\r\n$8\r\nsentinel\r\n");
            if((res = redis_send_command_and_read_res(fd->s_sock, buffer, buffer, strlen(buffer))) < 1)
            {
                goto next;
            }
            
            //使用正则取出ip和端口
            char dstbuf[1024];
            const char *pattern=".*address=(.*):(.*),slaves.*";             
            int num = g_regexec_get_value(buffer, pattern, dstbuf, 1024, 6);
            if(num < 1)
            {
               goto next;
            }
            strcpy(masterhost, dstbuf);
            strcpy(masterport, dstbuf + strlen(dstbuf) + 1);
            goto connectmaster;
        }
        else
        {
            //认证
            if(redis_auth_use_password(fd->s_sock, instance_info->server_password) != 0)
            {
                goto next;
            }
            dbgmsg("redis_connect_instance: auth to %s:%s success\n",masterhost, masterport);
            //发送info replication
            strcpy(buffer, "*2\r\n$4\r\ninfo\r\n$11\r\nreplication\r\n");
            if((res = redis_send_command_and_read_res(fd->s_sock, buffer, buffer, strlen(buffer))) < 1)
            {
                goto next;
            }
            //当前节点是master节点
            if(strstr(buffer,"role:master") > 0)
            {
                //已经找到
                goto ret;
            }
            else
            {
                //读取master_host:10.0.2.80\r\nmaster_port:6379
                //使用正则取出ip和端口
                char dstbuf2[1024];
                const char *pattern2 = ".*master_host:(.*)\r\nmaster_port:([0-9]{1,5})"; 
                
                int num = g_regexec_get_value(buffer, pattern2, dstbuf2, 1024, 6);
                if(num < 1)
                {
                   goto next;
                }
                
                strcpy(masterhost, dstbuf2);
                strcpy(masterport, dstbuf2 + strlen(dstbuf2) + 1);
                goto connectmaster;
            }
        }
next:        
        CLOSE_SOCKET(fd->s_sock);
        fd->s_sock = -1;
        masterhost[0] = '\0';
        masterport[0] = '\0';       
    }

connectmaster:
    //目前只连接一次
    if(strlen(masterhost) > 0 && strlen(masterport) > 0)
    {
        fd->s_sock = redis_connect_node(masterhost, masterport, instance_info->server_password);
        if(fd->s_sock < 1){
            res = -5;
        }
    }
    else
    {
        res = -6;
    }
    
ret:
    if(fd->s_sock > 0)
    {
        //填充session信息
        strcpy(session->account, instance_info->account);
        strncpy(session->dbname, "redis", sizeof(session->dbname));
        //strncpy(session->audit, instance_info->audit, sizeof(session->dbname));
        strcpy(session->instance_name, instance_info->instance_name);
        strcpy(session->dip, masterhost);
        session->dport = atoi(masterport);
        
        //填充packet信息
        strcpy(packet->auth_password, instance_info->auth_password);
        strcpy(packet->server_password, instance_info->server_password);
        if(strcasecmp(instance_info->audit,"yes"))
        {
            packet->audit = 0;
        }
        else
        {
            packet->audit = 1;
        }
        res = 0;
    }
    
    redis_cache_free_instance_info(instance_info);   
    return res;
}

int redis_proxy(session_t *session, socket_fd *fd, redis_packet *packet)
{
    char buffer[1024] = {'\0'};
    char tmp[128] = {'\0'};
    int ret = 0;
    
    //获取客户端认证密码
    if((ret = redis_client_auth(fd, buffer)) != 0)
    {
        return ret;   
    }
    //printf("client password:%s\n", buffer);
    //connect实例节点
    ret = redis_connect_instance(session, fd, buffer, packet);    
    
    //反馈客户端连接成功
    if(ret == 0)
    {
        //+OK\r\n;
        if(tcp_send(fd->c_sock, "+OK\r\n", strlen("+OK\r\n")) == -1){
            return -1;
        }
    }
    else
    {
        
        redis_get_errormsg(ret, tmp);
        sprintf(buffer, "-ERR %s\r\n", tmp);
        if(tcp_send(fd->c_sock, buffer, strlen(buffer)) == -1){
            return -1;
        }
    }
    return ret;

}

int redis_proc(session_t *session, SOCKET cs, char* raw, int len)
{
    socket_fd fd;
    fd.c_sock = cs;
    struct pollfd fds[2];
	const int REVENTS = POLLIN | POLLERR;
	int ready = 0;
	char buf[BUFSIZE];
	int n = 0;
	int res = 0;
	char errnomsg[1024];
	redis_packet *packet = NULL;
	
	//初始化解析器
	
	packet = redis_decode_init(session);
	//认证
    if((res = redis_proxy(session, &fd, packet)) != 0)
    {
        //发送认证错误消息        
        session->status = redis_get_error_session_status(res);
        redis_get_errormsg(res, errnomsg);
        send_auth_message(session, errnomsg);
        goto err;
    }
    //发送认证成功消息
    session->status = SESSION_AUTH_SUC;
    send_auth_message(session, "logon success");
    if (fd.s_sock < 0)
	{
		goto err;
	}
	
	packet->fd = &fd;
	
	fds[0].fd = fd.c_sock;
	fds[1].fd = fd.s_sock;
	fds[0].events = fds[1].events = POLLIN|POLLERR|POLLHUP;
    
    while(1)
	{
		ready = poll(fds, 2, 1000 * 20/*one 20 second*/);
		if (EINTR == ready)
			continue;
		if (0 > ready)
		{
			dbgmsg("poll failed. ready=%d", ready);
			goto err;
		}
		
		//上行
		if(fds[0].revents & REVENTS)
		{
		    //printf("up bufsize:%d\n",packet->req_command.size - packet->req_command.data_len);
			n = tcp_read(fds[0].fd, packet->req_command.buf, packet->req_command.size - packet->req_command.data_len);
			packet->req_command.buf[n] = '\0';
			//printf("up read size:%d\n",n);
			if(n > 0)
			{
			    packet->req_command.data_len += n;
			    packet->stream_dir = DIR_REQUEST;
			    if(redis_decode(session, packet) != 0){
			        dbgmsg("drop_buf eror");
					goto err;
			    }
                /*			    
			    if(tcp_send(fds[1].fd, packet->req_command.buf, n) == -1)
			    {
			        dbgmsg("drop_buf eror");
					goto err;
			    }
			    packet->req_command.data_len = 0;
			    */
			}
			else
			{
			    goto err;
			}
			
			if(packet && packet->audit)
			{
    			//解析上行数据
    			/*
    			packet->data = buf;
    			packet->data_len = n;
    			packet->stream_dir = DIR_REQUEST;
    			redis_decode(session, packet);
    			*/
    		}
		}
		//下行
		if(fds[1].revents & REVENTS)
		{
		    //printf("down bufsize:%d\n",packet->res_command.size - packet->res_command.data_len);
			n = tcp_read(fds[1].fd, packet->res_command.buf, packet->res_command.size - packet->res_command.data_len);
			packet->res_command.buf[n] = '\0';
			//printf("down read size:%d\n",n);
			if(n > 0)
			{
			    packet->res_command.data_len += n;
			    packet->stream_dir = DIR_RESPONSE;
			    if(redis_decode(session, packet) != 0){
			        dbgmsg("drop_buf eror");
					goto err;
			    }
			    
			    /*
			    if(tcp_send(fds[0].fd, buf, n) == -1)
			    {
			        dbgmsg("drop_buf eror");
					goto err;
			    }
			    packet->res_command.data_len = 0;
			    */
			}
			else
			{
			    goto err;   
			}
			if(packet && packet->audit)
			{
    			//解析上行数据
    			/*
    			packet->data = buf;
    			packet->data_len = n;
    			packet->stream_dir = DIR_RESPONSE;
    			redis_decode(session, packet);
    			*/
    		}
		}
	}
err:
    dbgmsg(" redis_proc exit");  
    if(session->status != SESSION_AUTH_ERR)
    {
        //发送退出消息
        send_logout_message(session, "");
    }
    //发送tcp连接关闭消息
    //send_servererror(session, session->account, session->instance_name, "tcpClose");
    
    redis_decode_fee(packet);
    return 0;
}