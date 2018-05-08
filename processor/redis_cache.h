#ifndef _REDIS_CACHE_H
#define _REDIS_CACHE_H

#include "stream.h"
/*
cache file content:
{"result":"success","message":[
 {
   "instance_name":"sdsds","account":"mytest","auth_password":"",
   "server_password":"dsads":"audit":"no","service_model":"ha",
   "nodes":[{"nodeid":"eeqwqe","host":"192.18.0.1","port":322,"sentinel":"no"}]
 }
]
}
*/
typedef struct _redis_node_info {
    char nodeid[36];
    char host[36];
    int port;
    char sentinel[4];//yes|no
} redis_node_info;

typedef struct _redis_instance_info {
    char instance_name[36];
    char account[36];
    char auth_password[36];
    char server_password[36];
    char audit[8];//yes|no
    char service_model[16];//sentinel|cluster|server|ha
    int node_num;
    redis_node_info *nodes;
} redis_instance_info;

/**
 * get instance_info by password
 * need free 
 */
redis_instance_info* redis_cache_get_instance_info(char *password);

void redis_cache_free_instance_info(redis_instance_info* instance_info);

int start_redis_cache_thread();

#endif
