#ifndef _MYSQL_CACHE_H
#define _MYSQL_CACHE_H

#include "stream.h"

typedef struct _server_info{
    char host[64];
    char port[8];
    char username[64];
    char password[64];
    char dbname[64];
    char instance_name[32];
    char audit[8];
} server_info;

int mysql_cache_get_md5(char *dst, unsigned char *data, int upperCase);
int mysql_cache_do_request(char *message, int len, char *url);

int start_mysql_cache_thread();
//int get_connect_host(char *username, server_info *serverinfo);
int mysql_cache_get_connect_host(char *username, server_info *serverinfo);

#endif
