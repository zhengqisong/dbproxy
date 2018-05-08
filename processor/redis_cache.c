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
#include <sys/stat.h>  
#include <openssl/md5.h>
#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "etcp.h"
#include "misc.h"
#include "sys_conf.h"
#include "g_string.h"
#include "redis_cache.h"
#include "g_cjson.h"

int start_redis_cache_thread(){
    if(strlen(sys_conf.control.cache_file) > 0){
        
        //if(launch_thread(loadUserAccountList, &ss, 0, 1024 * 32, NULL)){
        //    dbgmsg("start mysql cache thread error.");
        //    return -1;
        //}
        
    }
    return 0;
}

redis_instance_info* redis_cache_get_instance_info(char *password)
{
    FILE *fr = NULL;    
    char *buffer = NULL;
    char *p = NULL;
    char *access_key = sys_conf.control.access_key;
    char *cache_file = sys_conf.control.cache_file;
    cJSON * pJsonRoot = NULL;
    struct stat statbuf;
    int size,i,j,m;
    
    redis_instance_info *instance_info = NULL;
    redis_node_info *node_info_p;
    
    if(password == NULL || strlen(password) < 1)
    {
        goto ret;
    }
    
    fr = fopen(cache_file, "rb");
    if(fr == NULL)
    {
       goto ret;
    }
    
    stat(cache_file, &statbuf);  
    size = statbuf.st_size;  
    if(size < 1)
    {
       goto ret;
    }
    buffer = malloc(size + 10); 
    if(buffer == NULL){
        dbgmsg("can not malloc mem.");
        goto ret;
    }
    
    p = buffer;
    do
    {
       size = fread(p, 1, 1024, fr);
       if(size < 0) goto ret;
       p += size;
    }while(size > 0);
    //解密
    
    p = buffer;
    pJsonRoot = cJSON_Parse(p);
    if(pJsonRoot == NULL){
        dbgmsg("parser cache file error.");
        goto ret;
    }
    
    cJSON *messageJson = NULL;
    cJSON *instanceJson = NULL;  
    cJSON *nodesJson = NULL;  
    cJSON *nodeJson = NULL;  
    
    //result	        
    if(strcmp(cJSON_GetObjectItem(pJsonRoot, "result")->valuestring, "success"))
    {
        dbgmsg("Invalid cache file error.result=%s",cJSON_GetObjectItem(pJsonRoot, "result")->valuestring);
        goto ret;
    }
    messageJson = cJSON_GetObjectItem(pJsonRoot, "message");
    size = cJSON_GetArraySize(messageJson);
    for(i=0; i<size; i++)
    {
        instanceJson = cJSON_GetArrayItem(messageJson, i);
        if(cJSON_GetObjectItem(instanceJson, "instance_name") == NULL 
            || cJSON_GetObjectItem(instanceJson, "account") == NULL
            || cJSON_GetObjectItem(instanceJson, "server_password") == NULL 
            || cJSON_GetObjectItem(instanceJson, "audit") == NULL 
            || cJSON_GetObjectItem(instanceJson, "audit") == NULL  
            || cJSON_GetObjectItem(instanceJson, "service_model") == NULL){
            continue;
        }
        if(strcmp(cJSON_GetObjectItem(instanceJson, "auth_password")->valuestring, password))
            continue;
        nodesJson = cJSON_GetObjectItem(instanceJson, "nodes");
        m = cJSON_GetArraySize(nodesJson);
        if(m < 1) goto ret;
        
        //创建实例对象
        instance_info = (redis_instance_info *)malloc(sizeof(redis_instance_info) + m * sizeof(redis_node_info));
        instance_info->node_num = m;
        instance_info->nodes = (redis_node_info *)(instance_info + 1);
        
        
        strcpy(instance_info->instance_name, cJSON_GetObjectItem(instanceJson, "instance_name")->valuestring);
        strcpy(instance_info->account, cJSON_GetObjectItem(instanceJson, "account")->valuestring);
        strcpy(instance_info->auth_password, cJSON_GetObjectItem(instanceJson, "auth_password")->valuestring);
        strcpy(instance_info->server_password, cJSON_GetObjectItem(instanceJson, "server_password")->valuestring);
        strcpy(instance_info->audit, cJSON_GetObjectItem(instanceJson, "audit")->valuestring);
        strcpy(instance_info->service_model, cJSON_GetObjectItem(instanceJson, "service_model")->valuestring);
        
        for(j=0; j<m; j++)
        {
            node_info_p = instance_info->nodes + j;
            nodeJson = cJSON_GetArrayItem(nodesJson, j);
            if(cJSON_GetObjectItem(nodeJson, "nodeid")==NULL
                || cJSON_GetObjectItem(nodeJson, "host")==NULL
                || cJSON_GetObjectItem(nodeJson, "sentinel")==NULL
                || cJSON_GetObjectItem(nodeJson, "port")==NULL)
            {
                continue;
            }
            strcpy(node_info_p->nodeid, cJSON_GetObjectItem(nodeJson, "nodeid")->valuestring);
            strcpy(node_info_p->host, cJSON_GetObjectItem(nodeJson, "host")->valuestring);
            strcpy(node_info_p->sentinel, cJSON_GetObjectItem(nodeJson, "sentinel")->valuestring);
            node_info_p->port = cJSON_GetObjectItem(nodeJson, "port")->valueint;
            instance_info->node_num++;
        }
        break;
    }
ret:
    
    if(buffer) free(buffer);        
    if(fr) fclose(fr);
    return instance_info;
}

redis_instance_info* redis_cache_get_instance_info_test(char *password)
{
    redis_instance_info *instance_info;
    redis_node_info *node_info_p;
    // test code
    instance_info = (redis_instance_info *)malloc(sizeof(redis_instance_info) + 1 * sizeof(redis_node_info));
    
    instance_info->node_num = 1;
    instance_info->nodes = (redis_node_info *)(instance_info + 1);
        
    strcpy(instance_info->instance_name, "dsadwqewwewew");
    strcpy(instance_info->account, "mytest");
    strcpy(instance_info->auth_password, "houht123");
    strcpy(instance_info->audit, "no");
    strcpy(instance_info->service_model, "ha");
    
    node_info_p = instance_info->nodes;
    strcpy(node_info_p->nodeid,"dsafwfewqfew1");
    strcpy(node_info_p->host,"10.0.2.80");
    node_info_p->port = 63801;
    strcpy(node_info_p->sentinel,"yes");
    /*
    node_info_p++;
    strcpy(node_info_p->nodeid,"dsafwfewqfew2");
    strcpy(node_info_p->host,"10.0.2.80");
    node_info_p->port = 6380;
    strcpy(node_info_p->sentinel,"no");
    
    node_info_p++;
    strcpy(node_info_p->nodeid,"dsafwfewqfew3");
    strcpy(node_info_p->host,"10.0.2.80");
    node_info_p->port = 6379;
    strcpy(node_info_p->sentinel,"no");
    */
    return instance_info;
}

void redis_cache_free_instance_info(redis_instance_info* instance_info)
{
    if(instance_info != NULL)
        free(instance_info);
}
