/*
 * sys_conf.h
 *
 *  Created on: 2015-7-30
 *      Author: zhengqs
 */
 
#ifndef SYS_CONF_H_
#define SYS_CONF_H_


typedef struct _SYSLOG_SERVER
{
	char ip[64];
	int port;
} SYSLOG_SERVER;

typedef struct _EVENT_CONF
{
	char server_ip[64];
	int server_port;
	SYSLOG_SERVER syslog_server[5];
} EVENT_CONF;

typedef struct _CONTROL_CONF
{
    char type[32];
    char protocol[32];
    char url[128];
    char access_id[36];
    char access_key[36];
    char audit[8];
    char zone_ids[50];
    char cache_file[256];
} CONTROL_CONF;

typedef struct _SYS_CONF
{
    CONTROL_CONF control;
	EVENT_CONF event;
} SYS_CONF;

int sys_conf_init(char *config_file, SYS_CONF *sys_conf);

extern SYS_CONF sys_conf;

extern int rds_proxy_argc;
extern char **rds_proxy_argv;

#endif /* SYS_CONF_H_ */