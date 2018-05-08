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
#include <time.h>

#include "session.h"
#include "trace.h"
#include "etcp.h"
#include "sys_conf.h"

int preg_replace(char *dest, int dest_len, char *str, int str_len)
{
    int r = 0;
    int i = 0;   
    for(i = 0; i < str_len; i++)
    {
       if(str[i] == '"')
       {
          dest[r] = '\\';
          dest[r + 1] = '"';
          r += 2;
       }
       else if(str[i] == '\a')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'a';
          r += 3;
       }
       else if(str[i] == '\b')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'b';
          r += 3;
       }
       else if(str[i] == '\t')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 't';
          r += 3;
       }
       else if(str[i] == '\n')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'n';
          r += 3;
       }
       else if(str[i] == '\v')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'v';
          r += 3;
       }
       else if(str[i] == '\f')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'f';
          r += 3;
       }
       else if(str[i]=='\r')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          dest[r + 2] = 'r';
          r += 3;
       }
       else if(str[i]=='\\')
       {
          dest[r] = '\\';
          dest[r + 1] = '\\';
          r += 2;
       }
       else{
          dest[r] = str[i];
          r += 1;
       }
    }
    return r;
}

int send2udp(char *message)
{
    int index = 0;
    while(index < 5)
	{
	    if(sys_conf.event.syslog_server[index].ip[0] == 0)
		{
			break;
		}
		//dbgmsg("ip=%s,port=%d",sys_conf.event.syslog_server[index].ip,sys_conf.event.syslog_server[index].port);
		send_udp_msg(message, strlen(message), sys_conf.event.syslog_server[index].ip, sys_conf.event.syslog_server[index].port);
	    index++;
		
	}
	return 0;
}

int send_auth_message(session_t *session, char *message)
{
    char buf[1204] = {0};
    char json_msg[4096] = {0};
    struct tm when;
	time_t now;
	char s_time[64];
	char status[32];
	
    time(&now);
	when = *localtime(&now);
	sprintf(s_time, "%d-%02d-%02d %02d:%02d:%02d", 1900 + when.tm_year, 1 + when.tm_mon, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);
	
	if(session->status==SESSION_AUTH_ERR)
	{
	    strcpy(status, "autherror");
	}
	else if(session->status==SESSION_AUTH_SUC){
	    strcpy(status, "success");   
	}
	else if(session->status==SESSION_INTERFACE_ERR){
	    strcpy(status, "intererror");   
	}
	else if(session->status==SESSION_TCP_ERR){
	    strcpy(status, "tcperror");   
	}
	else if(session->status==SESSION_LOCK_ERR){
	    strcpy(status, "lockerror");   
	}
	else if(session->status==SESSION_OTHER_ERR){
	    strcpy(status, "othererror");   
	}
	else{
	    strcpy(status, "unknow"); 
	}
	preg_replace(buf, sizeof(buf), message, strlen(message));
	
    sprintf(json_msg, "{\"node\":\"%s\",\"device\":\"rdsProxy\",\"module\":\"auth\",\"time\":\"%s\",\"message\":{\
\"protocol\":\"%s\", \
\"sessionId\":\"%s\", \
\"account\":\"%s\", \
\"sip\":\"%s\", \
\"sport\":\"%d\", \
\"status\":\"%s\", \
\"instanceName\":\"%s\", \
\"dip\":\"%s\", \
\"dport\":\"%d\", \
\"dbname\":\"%s\", \
\"message\":\"%s\", \
\"authTime\":\"%s\" \
}}", session->proxy_id, s_time, sys_conf.control.protocol, session->session_id, session->account, session->sip, session->sport, status, session->instance_name
    , session->dip, session->dport, session->dbname, buf, s_time);
    
    send2udp(json_msg);
    
    dbgmsg("send auth message: %s", json_msg);
    return 0;
}
int send_logout_message(session_t *session, char *message)
{
    char buf[1204] = {0};
    char json_msg[4096] = {0};
    struct tm when;
	time_t now;
	char s_time[64];
	
    time(&now);
	when = *localtime(&now);
	sprintf(s_time, "%d-%02d-%02d %02d:%02d:%02d", 1900 + when.tm_year, 1 + when.tm_mon, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);
	
	
	preg_replace(buf, sizeof(buf), message, strlen(message));
	
    sprintf(json_msg, "{\"node\":\"%s\",\"device\":\"rdsProxy\",\"module\":\"logout\",\"time\":\"%s\",\"message\":{\
\"protocol\":\"%s\", \
\"sessionId\":\"%s\", \
\"account\":\"%s\", \
\"sip\":\"%s\", \
\"sport\":\"%d\", \
\"instanceName\":\"%s\", \
\"dip\":\"%s\", \
\"dport\":\"%d\", \
\"dbname\":\"%s\", \
\"message\":\"%s\", \
\"logoutTime\":\"%s\" \
}}", session->proxy_id, s_time, sys_conf.control.protocol, session->session_id, session->account, session->sip, session->sport, session->instance_name
    , session->dip, session->dport, session->dbname, buf, s_time);
    
    send2udp(json_msg);
    
    dbgmsg("send logout message: %s", json_msg);
    return 0;
}
int send_audit_message(session_t *session, char *result, float spend_time, char *sql, char *message)
{
    
    
    char buf[1204] = {0};
    char sql_buf[1204] = {0};
    
    char json_msg[4096] = {0};
    struct tm when;
	time_t now;
	char s_time[64];
	
    time(&now);
	when = *localtime(&now);
	sprintf(s_time, "%d-%02d-%02d %02d:%02d:%02d", 1900 + when.tm_year, 1 + when.tm_mon, when.tm_mday, when.tm_hour, when.tm_min, when.tm_sec);
	
	
	preg_replace(buf, sizeof(buf), message, strlen(message));
	preg_replace(sql_buf, sizeof(sql_buf), sql, strlen(sql));
	
	
    sprintf(json_msg, "{\"node\":\"%s\",\"device\":\"rdsProxy\",\"module\":\"audit\",\"time\":\"%s\",\"message\":{\
\"protocol\":\"%s\", \
\"sessionId\":\"%s\", \
\"account\":\"%s\", \
\"dbname\":\"%s\", \
\"instanceName\":\"%s\", \
\"sql\":\"%s\", \
\"result\":\"%s\", \
\"spendTime\":\"%.3f\", \
\"message\":\"%s\", \
\"time\":\"%s\" \
}}", session->proxy_id, s_time, sys_conf.control.protocol, session->session_id, session->account, session->dbname, session->instance_name
    , sql_buf, result, spend_time, buf, s_time);
    
    send2udp(json_msg);
    
    dbgmsg("send audit message: %s", json_msg);
    return 0;
}