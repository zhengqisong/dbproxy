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
#include "my_global.h"
#include "my_sha1.h"
#include "proto_parser_conf.h"
#include "mysql_parser.h"
#include "g_string.h"

#include "mysql_cache.h"

#include "g_xml.h"
#include "session.h"

#define BUFSIZE	1024
//#define __MYSQL_THREAD__

char *salt_char = "abcdefghijklmnopqrstuvwxyz!@#$%^&*()_+<>;1234567890";
char *synchro_key = "w8w032l3$#ewe.eo3";
static int rand_init = 0;
/*----------------common---------------------------*/



/**
 * 向api server 发送错误信息
 **/
int mysql_send_servererror(session_t *session, char *username, char *instanceName, char* errroType)
{
    time_t rawtime;
    char *type = sys_conf.control.type;
    char *url = sys_conf.control.url;
    char *access_id = sys_conf.control.access_id;
    char *access_key = sys_conf.control.access_key;
    
    char param[1024] = {'\0'};
    char sign[128] = {'\0'};
    char buf[2048] = {'\0'};
    char tmp[2048] = {'\0'};
    //请求地址和参数3.24 2494.7
    time ( &rawtime );
    if(strcmp(type, "synchro") != 0)
    {
        sprintf(tmp, "%lu%s%s", rawtime, access_id, access_key);
        mysql_cache_get_md5(sign, tmp, 1);
        sprintf(param, "format=xml&username=%s&timestamp=%lu&access_id=%s&sign=%s&errroType=%s&instanceName=%s&sessionId=%s&sip=%s", username, rawtime, access_id, sign, errroType, instanceName, session->session_id, session->sip);
        
        sprintf(tmp, "curl --data \"%s\" %s%s", param, url, "/instance/userdberror");
        mysql_cache_do_request(buf, sizeof(buf), tmp);
        return 0;
    }
    return -1;
}

int mysql_get_serverinfo(session_t *session, char *username, server_info *serverinfo)
{
    char *type = sys_conf.control.type;
    char *url = sys_conf.control.url;
    char *access_id = sys_conf.control.access_id;
    char *access_key = sys_conf.control.access_key;
    char *audit = sys_conf.control.audit;
    
    char param[1024] = {'\0'};
    char sign[128] = {'\0'};
    char buf[2048] = {'\0'};
    char tmp[2048] = {'\0'};
    time_t rawtime;
    
    char *p = NULL;
    
    xmlDocPtr doc;
	int status = 0, code = 0, _index = -1, res = 0;
	
    /*
    **代理类型分三种
    ** 1) 可读写代理，通过账号向rds api获取服务器地址
    ** 2) 只读代理，通过账号向rds api获取服务器地址
    ** 3) 同步代理，通过分析账号获取port，目标ip为本地ip
    */
	//------------------账号=port_账号--------------------------------
	memset(serverinfo, 0, sizeof(server_info));
	if(strcmp(type, "synchro") == 0)
	{
	    //memcpy(serverinfo->host, tmp, strlen(tmp));
	    //memcpy(serverinfo->port, tmp, strlen(tmp));
	    //memcpy(serverinfo->username, tmp, strlen(tmp));
	    //memcpy(serverinfo->password, tmp, strlen(tmp));
	    //memcpy(serverinfo->audit, tmp, strlen(tmp));
	    
	    p = strstr(username, "_");
	    if(p == NULL){
	        return -3;
	    }
	    _index = p - username;
	    if(_index < 2)
	    {
	        return -3;
	    }
	    //memcpy(serverinfo->host, "172.16.1.120", strlen("172.16.1.120"));
	    memcpy(serverinfo->host, session->proxy_ip, strlen(session->proxy_ip));
	    memcpy(serverinfo->port, username, _index);
	    memcpy(serverinfo->username, username + _index + 1, strlen(username) - _index - 1);
	    sprintf(tmp, "%s%s", serverinfo->username, synchro_key);
	    mysql_cache_get_md5(serverinfo->password, tmp, 0);
	    memcpy(serverinfo->audit, audit, strlen(audit));
	    
	    return 0;
	}
	
	//--------------------------------------------------
	
	
    //请求地址和参数3.24 2494.7
    time ( &rawtime );
    
    sprintf(tmp, "%lu%s%s", rawtime, access_id, access_key);
    mysql_cache_get_md5(sign, tmp, 1);
    sprintf(param, "format=xml&username=%s&timestamp=%lu&access_id=%s&sign=%s&type=%s&sessionId=%s&sip=%s", username, rawtime, access_id, sign, type, session->session_id, session->sip);
    
    sprintf(tmp, "curl --data \"%s\" %s%s", param, url, "/instance/userdbinfo");
    mysql_cache_do_request(buf, sizeof(buf), tmp);
    
    //load xml
	if(xml_parse_memory(buf, strlen(buf), &doc))
	{
	    dbgmsg_ex(TRACE_ID_9, "parser xml error, xml:%s", buf);
	    res = -2;
	    //return -2;
	    goto loadfromlocal;
	}
    
    xml_get_str_by_path_key2(doc, "/res/status", tmp, sizeof(tmp), "-1");
	status = atoi(tmp);
	xml_get_str_by_path_key2(doc, "/res/code", tmp, sizeof(tmp), "-1");
	code = atoi(tmp);
	if(status != 0 )
	{
	    xml_get_str_by_path_key2(doc, "/res/message", tmp, sizeof(tmp), "-1");
	    if(code == 8000){
	        //dbgmsg_ex(TRACE_ID_9, "interface right error, message:%s", buf);
	        res = -2;
            return -2;	    
	    }
	    if(code == 4000){
	        //dbgmsg_ex(TRACE_ID_9, "interface right error, message:%s", buf);
	        res = -4;
            return -4;   
	    }
	    if(code == 3000){
	        //dbgmsg_ex(TRACE_ID_9, "interface right error, message:%s", buf);
	        res = -7;
            return -7;   
	    }
	    if(code == 300){
	        goto  loadfromlocal;  
	    }
	    res = -3;
	    return -3;
	}
	
	//赋值	
	xml_get_str_by_path_key2(doc, "/res/message/host", tmp, sizeof(tmp), "");
	memcpy(serverinfo->host, tmp, strlen(tmp));
	memset(tmp, 0, 20);
	xml_get_str_by_path_key2(doc, "/res/message/port", tmp, sizeof(tmp), "");
	memcpy(serverinfo->port, tmp, strlen(tmp));
	xml_get_str_by_path_key2(doc, "/res/message/username", tmp, sizeof(tmp), "");
	memcpy(serverinfo->username, tmp, strlen(tmp));
	xml_get_str_by_path_key2(doc, "/res/message/password", tmp, sizeof(tmp), "");
	memcpy(serverinfo->password, tmp, strlen(tmp));
	xml_get_str_by_path_key2(doc, "/res/message/instanceName", tmp, sizeof(tmp), "");
	memcpy(serverinfo->instance_name, tmp, strlen(tmp));
	xml_get_str_by_path_key2(doc, "/res/message/dbname", tmp, sizeof(tmp), "");
	memcpy(serverinfo->dbname, tmp, strlen(tmp));
	
	xml_get_str_by_path_key2(doc, "/res/message/audit", tmp, sizeof(tmp), "");
	memcpy(serverinfo->audit, tmp, strlen(tmp));
	
	return 0;
    
loadfromlocal:
    dbgmsg("load user info from local cache file,%s", username);
    res = mysql_cache_get_connect_host(username, serverinfo);
/*    dbgmsg("this is runing....host:%s,port:%s,username:%s,password:%s,instance_name:%s,dbname:%s,audit:%s", serverinfo->host,serverinfo->port, serverinfo->username \
        , serverinfo->password, serverinfo->instance_name, serverinfo->dbname,serverinfo->audit);
*/
    return res;
}

int mysql_connect_server(server_info *serverinfo)
{
    SOCKET s_sock;
    int enable = 1;
    //dbgmsg("to connect db server");
    s_sock = tcp_client(serverinfo->host, serverinfo->port, 1, fatal_quit_f);
	if (s_sock < 0)
	{
		dbgmsg("connect server error server:(%s,%s)", serverinfo->host, serverinfo->port);
		return -1;
	}
	setsockopt(s_sock, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    
	return s_sock;
}
/*-------------------------------------------*/

int mysql_get_salt(char *salt,int len)
{
    int i;
    int num;
    
    if(rand_init == 0)
    {
       rand_init = 1;
       srand(time(0));   
    }
    for(i=0; i<len; i++)
    {
       num = rand()%strlen(salt_char);
       salt[i] = salt_char[num]; 
    }
    return 0;
}

int mysql_check_password(mysql_server_greeting *server_greeting, msyql_logon_request *logon_request, server_info *serverinfo)
{
    unsigned char message[20];
    unsigned char to[50];
    //dbgmsg(serverinfo->password);
    
    //验证用户账号和密码是否正确
    memcpy(message, server_greeting->salt1, 8);
    memcpy(message+8, server_greeting->salt2, 12);
    
    scramble(to, message, serverinfo->password);
    if(logon_request->password_len<=1){
        return -4;
    }
    if(memcmp(to, logon_request->password, logon_request->password_len) != 0)
    {
        //dbgmsg("password not correct");
		return -4;
    }
    return 0;
}

int mysql_client_auth(socket_fd *fd, mysql_server_greeting *server_greeting, msyql_logon_request *logon_request)
{
    char buf[1024];
    char *p;
    char *s;
    char *server_version = "8.0.0-rdsProxy";
    
    int packet_len;
    int packet_num = 0;

    
    server_greeting->protocol = 10;
    strcpy(server_greeting->version, server_version);
    server_greeting->thread_id = 343279;
    mysql_get_salt(server_greeting->salt1, 8);
    mysql_get_salt(server_greeting->salt2, 12);
    server_greeting->capabilities = 0xf7ff;
    server_greeting->charset = 8;
    server_greeting->status = 0x0002; 
    
    //1.0 create server greeting packet
    p = buf + 4;
    mysql_out_uint8(p, server_greeting->protocol);
    mysql_out_string_null(p, server_greeting->version);
    mysql_out_uint32_le(p, server_greeting->thread_id);
    mysql_out_uint8p(p, server_greeting->salt1, 8);
    mysql_out_uint8s(p, 1);
    mysql_out_uint16_le(p, server_greeting->capabilities);
    mysql_out_uint8(p, server_greeting->charset);
    mysql_out_uint16_le(p, server_greeting->status);
    //--------------------------------------
    //mysql_out_uint8s(p, 13);
    //7f 80 15 00 00 00 00 00 00 00 00 00 00
    //this is can not support jdbc >= 5.1.44
    mysql_out_uint8(p, 0xff); 
    //mysql_out_uint8(p, 0x7f);
    mysql_out_uint8(p, 0xc1); 
    //mysql_out_uint8(p, 0x80);
    mysql_out_uint8(p, 0x15);
    mysql_out_uint8s(p, 10);
    //--------------------------
    mysql_out_uint8p(p, server_greeting->salt2, 12);
    mysql_out_uint8s(p, 1);
    mysql_out_string_null(p, "mysql_native_password");
    
    packet_len = p - buf - 4;
    p = buf;
    mysql_out_uint24_le(p, packet_len);
    mysql_out_uint8(p, packet_num);
    packet_len += 4;
    
    //1.1 send packet to client
    if(tcp_send(fd->c_sock, buf, packet_len) == -1){
        return -1;
    }
    
    //1.2 read logon packet
    if(readn(fd->c_sock, buf, 4) == -1){
        return -1;   
    }
    
    p = buf;
    mysql_in_uint24_le(p, packet_len);
    //dbgmsg("packet type = %d", (u_int8_t*)buf[3]);
    if(readn(fd->c_sock, buf + 4, packet_len) == -1){
        return -1;
    }
    
    //1.3 logon request analysis
    p = buf + 4;
    mysql_in_uint16_le(p, logon_request->capabilities);
    mysql_in_uint16_le(p, logon_request->ex_capabilities);
    mysql_in_uint32_le(p, logon_request->max_packet);
    mysql_in_uint8(p, logon_request->charset);
    mysql_in_uint8p(p, logon_request->resvered, 23);
    s = logon_request->username;
    mysql_in_string_null(p, s);
    mysql_in_uint8(p, logon_request->password_len);
    mysql_in_uint8p(p, logon_request->password, logon_request->password_len)
    
    //other_payload_len=-203,packet_len=87
    //CLIENT_LOCAL_FILES  128
    //CLIENT_INTERACTIVE 1024  
    logon_request->other_payload_len = packet_len - (p - (buf + 4));
    //dbgmsg("other_payload_len=%d,packet_len=%d, in_len=%d,passwd_len=%d",logon_request->other_payload_len, packet_len, p - (buf + 4), logon_request->password_len);  
    memcpy(logon_request->other_payload, p, logon_request->other_payload_len);
    
    //dbgmsg("p-buf=%d",p-buf);
     
    s = (buf + 4 + packet_len - 1);
    if(*s == 0 && packet_len - (p - (buf + 4)) > 0){
        s = logon_request->dbname;
        mysql_in_string_null(p, s);
    }
    //dbgmsg("client auth over");
    return 0;
}

int mysql_server_auth(socket_fd *fd, mysql_server_greeting *s_server_greeting, msyql_logon_request *s_logon_request, server_info *serverinfo)
{
    char buf[1024];
    char buf2[1024];
    char buf3[1024];
    
    char *p;
    char *s;
    
    int packet_len;
    unsigned char message[20];
    unsigned char to[50];
    
    unsigned int num;
    unsigned short error_code;
    
    //2.2 读取服务端的greeting packet
    if(readn(fd->s_sock, buf, 4) == -1){
        return -1;   
    }
    p = buf;
    mysql_in_uint24_le(p, packet_len);
    //dbgmsg("server greeting packet_len=%d",packet_len);
    if(readn(fd->s_sock, buf, packet_len) == -1){
        return -1;
    }
    //dbgmsg("server greeting run..1");
    //2.3 分析greeting packet
    
    mysql_in_uint8(p, s_server_greeting->protocol);
    s = s_server_greeting->version;
    mysql_in_string_null(p, s);
    mysql_in_uint32_le(p, s_server_greeting->thread_id);
    s = s_server_greeting->salt1;
    mysql_in_string_null(p, s);
    mysql_in_uint16_le(p, s_server_greeting->capabilities);
    mysql_in_uint8(p, s_server_greeting->charset);
    mysql_in_uint16_le(p, s_server_greeting->status);
    mysql_in_uint8p(p, s_server_greeting->reverve, 13);
    s = s_server_greeting->salt2;
    mysql_in_string_null(p, s);
    
    //2.4 创建logon request
    memset(s_logon_request->username, 0 , sizeof(s_logon_request->username));
    memcpy(s_logon_request->username, serverinfo->username, strlen(serverinfo->username));
    
    memcpy(message, s_server_greeting->salt1, 8);
    memcpy(message+8, s_server_greeting->salt2, 12);
    memset(to, 0 , sizeof(to));
    scramble(to, message, serverinfo->password);
    
    //dbgmsg("message=%s,password=%s", message, serverinfo->password);
    s_logon_request->password_len = 20;
    memcpy(s_logon_request->password, to, s_logon_request->password_len);
    
    //2.5创建认证 buf
    p = buf + 4;
    mysql_out_uint16_le(p, s_logon_request->capabilities);
    mysql_out_uint16_le(p, s_logon_request->ex_capabilities);
    mysql_out_uint32_le(p, s_logon_request->max_packet);
    mysql_out_uint8(p, s_logon_request->charset);
    mysql_out_uint8p(p, s_logon_request->resvered, 23);
    mysql_out_string_null(p, s_logon_request->username);
    mysql_out_uint8(p, s_logon_request->password_len);
    mysql_out_uint8p(p, s_logon_request->password, s_logon_request->password_len);
//    if(strlen(s_logon_request->dbname)>0){
//       mysql_out_string_null(p, s_logon_request->dbname);
//    }
    if(s_logon_request->other_payload_len > 0){
        memcpy(p, s_logon_request->other_payload, s_logon_request->other_payload_len);
        p += s_logon_request->other_payload_len;
    }
    packet_len = p - buf - 4;
    p = buf;
    mysql_out_uint24_le(p, packet_len);
    mysql_out_uint8(p, 1);
    packet_len += 4;
    
    //dbgmsg("packet_len=%d", packet_len);
    //2.6 发送logon request
    if(tcp_send(fd->s_sock, buf, packet_len) == -1){
        return -1;
    }
    num = packet_len;
    
    //3.1接收服务端的连接结果
    memset(buf2, 0, sizeof(buf2));
    if(readn(fd->s_sock, buf2, 4) == -1){
        return -1;   
    }
    p = buf2;
    mysql_in_uint24_le(p, packet_len);
    //dbgmsg("server response packet_len=%d",packet_len);
    if(readn(fd->s_sock, buf2 + 4, packet_len) == -1){
        return -1;
    }
    if(buf2[4] == 0xFF){
      //返回错误信息
      byte2hexchar(buf3, buf, num);
      dbgmsg("error request packet len=%d, =%s", num*2, buf3);
      p = buf2 + 5;
      mysql_in_uint16_le(p, error_code);
      dbgmsg("%s auth to server error, code=%d, message:%s", s_logon_request->username, error_code, buf2 + 13);
    }
    //else{
    //  byte2hexchar(buf3, buf, num);
    //  dbgmsg("request packet len=%d, =%s", num*2, buf3);   
    //}
    //3.2转发服务端的请求结果
    if(tcp_send(fd->c_sock, buf2, packet_len + 4) == -1){
        return -1;
    }
    return 0;
}

int mysql_check_auth(socket_fd *fd)
{
    return 0;
}

int mysql_proxy(session_t *session, socket_fd *fd, mysql_packet *packet)
{
    int ret = 0,connect_number = 0;
    //发送给客户端的代理服务信息
    mysql_server_greeting server_greeting;
    //接收到的客户端认证信息
    msyql_logon_request logon_request;
    //接收到的mysql server服务信息
    mysql_server_greeting s_server_greeting;
    //向服务端登录认证信息
    msyql_logon_request s_logon_request;
    server_info serverinfo;
    
    //认证客户端
    memset(&server_greeting, 0, sizeof(mysql_server_greeting));
    memset(&logon_request, 0, sizeof(msyql_logon_request));
    //dbgmsg("logon_request.dbname1=%s",logon_request.dbname);
    if((ret = mysql_client_auth(fd, &server_greeting, &logon_request)) != 0)
    {
        return ret;   
    }
    //dbgmsg("logon_request.dbname2=%s",logon_request.dbname);
    //dbgmsg("session->dbname1=%s",session->dbname);
    strcpy(session->account, logon_request.username);
    strncpy(session->dbname, logon_request.dbname, sizeof(session->dbname));
    //dbgmsg("session->dbname2=%s",session->dbname);

connect:    
    //获取服务器信息
    if((ret = mysql_get_serverinfo(session, logon_request.username, &serverinfo)) !=0 )
    {
        return ret;
    }
    
    //设置客户端服务能力
    if(packet)
    {
        packet->client_capablites = logon_request.capabilities;
        if(strcmp("yes", serverinfo.audit) == 0)
            packet->audit = 1;
        else
            packet->audit = 0;
    }
    
    strcpy(session->dip, serverinfo.host);
    session->dport = atoi(serverinfo.port);
    strcpy(session->instance_name, serverinfo.instance_name);
    
    //验证客户端请求的密码是否正确
    if((ret = mysql_check_password(&server_greeting, &logon_request, &serverinfo)) !=0 )
    {
        return ret;
    }
    //关闭socket
    if (isvalidsock(fd->s_sock))
	{
	    CLOSE_SOCKET(fd->s_sock);
		fd->s_sock = -1;
	}
	//连接服务器       
    fd->s_sock = mysql_connect_server(&serverinfo);
    if(fd->s_sock < 0)
    {
        //最多连接3次
        if(connect_number < 3)
        {
            connect_number++;
            goto connect;
        }
        //发送连接错误消息
        mysql_send_servererror(session, logon_request.username, serverinfo.instance_name, "tcpError");
        return -5;
    }
    //发送tcp连接成功消息
    mysql_send_servererror(session, logon_request.username, serverinfo.instance_name, "tcpSuccess");
    
    //服务端认证
    memset(&s_server_greeting, 0, sizeof(mysql_server_greeting));
    memset(&s_logon_request, 0, sizeof(msyql_logon_request));
    memcpy(&s_logon_request, &logon_request, sizeof(msyql_logon_request));
    if((ret = mysql_server_auth(fd, &s_server_greeting, &s_logon_request, &serverinfo)) != 0)
    {
        //发送认证错误消息
        if(mysql_send_servererror(session, logon_request.username, serverinfo.instance_name, "accountError")==0)
        {
            connect_number++;
            goto connect;
        }
        return ret;
    }
    
    //判断认证结果是否正确
    if((ret = mysql_check_auth(fd)) != 0)
    {
        return -6;   
    }
    
    return 0;
}

int mysql_get_errormsg(int err, char *buf)
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
        strcpy(buf, "Account or password error");
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

int mysql_get_error_session_status(int err)
{
    dbgmsg("error %d",err);
    switch(err)
    {
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

int mysql_writen(int fd,const void* vptr, int n)
{
    int nleft;
    int nwritten;
    const char *ptr;
    ptr = (char*)vptr;
    nleft = n;

    while(nleft > 0) {
        if( (nwritten = write(fd,ptr,nleft))  <= 0) {
                if(errno == EINTR)
                        nwritten = 0;
                else
                        return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;
}

void *mysql_trans(void *arg)
{
    struct mysql_thread_info *tinfo = (struct mysql_thread_info *) arg;
    char buf[tinfo->bufsize];
    int nRead, nWrite;
    while(1) {
        nRead = read(tinfo->f_socket, buf, tinfo->bufsize);
        if(nRead < 0) {
            if(errno == EINTR) continue;
            dbgmsg("read error %d, nRead=%d, %s",tinfo->stream_dir, nRead, strerror(errno));
            break;
        }
        if(nRead == 0) { // remote close
            dbgmsg("read error nRead=0");
            break;
        }
        
        nWrite = mysql_writen(tinfo->t_socket, buf, nRead);
        if(nWrite < 0 || nWrite != nRead) {
            dbgmsg("write error %d, write=%d,read=%d %s",tinfo->stream_dir, nWrite, nRead);
            break;
        }
        /*
        if(tinfo->package && tinfo->package->audit)
		{
		    dbgmsg("audit error %d", tinfo->stream_dir);
			tinfo->package->data = tinfo->buf;
			tinfo->package->data_len = nRead;
			tinfo->package->stream_dir = tinfo->stream_dir;
			mysql_decode(tinfo->session, tinfo->package);
			dbgmsg("audit over %d", tinfo->stream_dir);
		}
		*/
    }
    close(tinfo->f_socket);
    close(tinfo->t_socket);

    //exit(0);
    pthread_exit(0);
}

int mysql_proc(session_t *session, SOCKET cs, char* raw, int len)
{
    socket_fd fd;
    fd.c_sock = cs;
    fd.s_sock = -1;
    
	int res = 0;
	char errnomsg[1024];
	mysql_packet *packet = NULL;
	struct mysql_thread_info *tinfo;
	
	//解决延迟40ms的问题
	int enable = 1;
    setsockopt(fd.c_sock, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));
    
	//初始化解析器
	packet = mysql_decode_init(session);
	//dbgmsg("mysql_proc:sys_conf.control.url=%s", sys_conf.control.url);
	//认证
    if((res = mysql_proxy(session, &fd, packet)) != 0)
    {
        //发送认证错误消息        
        session->status = mysql_get_error_session_status(res);
        mysql_get_errormsg(res, errnomsg);
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
	
	
#ifdef __MYSQL_THREAD__
    char request_buf[BUFSIZE];
    char respose_buf[BUFSIZE];
    
    
	tinfo = calloc(2, sizeof(struct mysql_thread_info));
	//tinfo[0].package = packet;
	//tinfo[0].session = session;
	tinfo[0].stream_dir = DIR_REQUEST;
	tinfo[0].f_socket = fd.c_sock;
	tinfo[0].t_socket = fd.s_sock;
	//tinfo[0].buf = request_buf;
	tinfo[0].bufsize = BUFSIZE;
	
	//tinfo[1].package = packet;
	//tinfo[1].session = session;
	tinfo[1].stream_dir = DIR_RESPONSE;
	tinfo[1].f_socket = fd.s_sock;
	tinfo[1].t_socket = fd.c_sock;
	//tinfo[1].buf = respose_buf;
	tinfo[1].bufsize = BUFSIZE;
	//if(launch_thread(mysql_trans, &tinfo[0], PTHREAD_CREATE_JOINABLE, 1024 * 32, &tinfo[0].thread_id)){
    if(pthread_create(&tinfo[0].thread_id, NULL, mysql_trans, &tinfo[0])){
        dbgmsg("start mysql proxy request thread error.");
        free(tinfo);
        goto err;
    }
    //if(launch_thread(mysql_trans, &tinfo[1], PTHREAD_CREATE_JOINABLE, 1024 * 32, &tinfo[1].thread_id)){
    if(pthread_create(&tinfo[1].thread_id, NULL, mysql_trans, &tinfo[1])){
        dbgmsg("start mysql proxy response thread error.");
        free(tinfo);
        goto err;
    }
    pthread_join(tinfo[0].thread_id, NULL);
    pthread_join(tinfo[1].thread_id, NULL);
    free(tinfo);
#else
    struct pollfd fds[2];
	const int REVENTS = POLLIN | POLLERR;
	int ready = 0;
	char buf[BUFSIZE];
	int n = 0;                         
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
			n = tcp_read(fds[0].fd, buf, BUFSIZE);
			
			if(n > 0)
			{
			    if(tcp_send(fds[1].fd, buf, n) == -1)
			    {
			        dbgmsg("drop_buf eror");
					goto err;
			    }
			}
			else
			{
			    goto err;
			}
			
			if(packet && packet->audit)
			{
    			//解析上行数据
    			packet->data = buf;
    			packet->data_len = n;
    			packet->stream_dir = DIR_REQUEST;
    			mysql_decode(session, packet);
    		}
		}
		//下行
		if(fds[1].revents & REVENTS)
		{
			n = tcp_read(fds[1].fd, buf, BUFSIZE);
			if(n > 0)
			{
			    if(tcp_send(fds[0].fd, buf, n) == -1)
			    {
			        dbgmsg("drop_buf eror");
					goto err;
			    }
			}
			else
			{
			    goto err;   
			}
			if(packet && packet->audit)
			{
    			//解析上行数据
    			packet->data = buf;
    			packet->data_len = n;
    			packet->stream_dir = DIR_RESPONSE;
    			mysql_decode(session, packet);
    		}
		}
	}
#endif

err:
    dbgmsg(" general_proc exit");    
    if(session->status != SESSION_AUTH_ERR)
    {
        //发送退出消息
        send_logout_message(session, "");
    }
    
    //发送tcp连接关闭消息
    mysql_send_servererror(session, session->account, session->instance_name, "tcpClose");
    
    mysql_decode_fee(packet);
    
    //关闭socket
    if (isvalidsock(fd.s_sock))
	{
	    CLOSE_SOCKET(fd.s_sock);
	    errno = 0;
		fd.s_sock = -1;
	}
    return 0;
}
