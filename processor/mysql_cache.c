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
#include "g_string.h"
#include "mysql_cache.h"
#include "g_xml.h"

int mysql_cache_get_md5(char *dst, unsigned char *data, int upperCase)
{
    unsigned char md[16];
    char tmp[3]={'\0'};
    int i;
    MD5_CTX ctx;

    MD5_Init(&ctx);
    MD5_Update(&ctx, data, strlen(data));
    MD5_Final(md,&ctx);

    for( i=0; i<16; i++ )
    {
        if(upperCase==1){
            sprintf(tmp,"%02X",md[i]);
        }else{
            sprintf(tmp,"%02x",md[i]);
        }
        strcat(dst,tmp);
    }
    return 0;
}

int mysql_cache_do_request(char *message, int len, char *url)
{
    FILE   *stream;
    
    stream = popen(url, "r" );
    fread(message, sizeof(char), len, stream);
    
    return 0;
}

int mysql_cache_getDecrypt(char *buff, char *password, char *key){
    int m = 0;
    int i=0;
    char x[2];
    char *p = buff;   
    for(i=0; i< strlen(password); i+=2){
        x[0] = password[i];
        x[1] = password[i+1]; 
        short z = strtol((char*)x, NULL, 16);
        z = z ^ key[m];
        *p = z & 0x0ff;
        p++;
        m = m+1;
        if(m == strlen(key)){
            m = 0;
        }
    }
    *p ='\0';
    
    return 0;
}

static void mysql_cache_process_load(){
    char *tmp = NULL;
    char *buf = NULL;
    char sign[128] = {'\0'};
    char param[1024] = {'\0'};
    int bufsize = 1024*1024*10;
    
    char *type = sys_conf.control.type;
    char *url = sys_conf.control.url;
    char *access_id = sys_conf.control.access_id;
    char *access_key = sys_conf.control.access_key;
    char *zone_ids = sys_conf.control.zone_ids;
    char *cache_file = sys_conf.control.cache_file;
    
    FILE *fw = NULL;
    xmlDocPtr doc;
    int status = 0, code = 0;
    
    tmp = malloc(bufsize); 
    buf = malloc(bufsize); 
    if(tmp == NULL || buf == NULL)
    {
        dbgmsg("can not malloc mem.");
        goto ret;
    }
    time_t rawtime;
    time ( &rawtime );
    
    memset(tmp, 0, bufsize);
    memset(sign, 0, sizeof(sign));
    memset(buf, 0, bufsize);
    memset(param, 0, sizeof(param));
    if(strlen(cache_file) < 1)
    {
        goto ret;
    }
    
    sprintf(tmp, "%lu%s%s", rawtime, access_id, access_key);
    
    mysql_cache_get_md5(sign, tmp, 1);
    
    sprintf(param, "format=xml&timestamp=%lu&access_id=%s&sign=%s&type=%s&zoneIds=%s", rawtime, access_id, sign, type, zone_ids);
    
    sprintf(tmp, "curl --data \"%s\" %s%s", param, url, "/instance/userdblist");
    //dbgmsg("curl:%s", tmp);
    mysql_cache_do_request(buf, bufsize, tmp);
    //dbgmsg("buf:%s",buf);
    mysql_cache_getDecrypt(tmp, buf, access_key);
    //dbgmsg("tmp:%s",tmp);
    if(xml_parse_memory(tmp, strlen(tmp), &doc))
    {
	    dbgmsg("load user account list. parser xml error.");
	    goto ret;
	}
    xml_get_str_by_path_key2(doc, "/res/status", tmp, bufsize, "-1");
	status = atoi(tmp);
	xml_get_str_by_path_key2(doc, "/res/code", tmp, bufsize, "-1");
	code = atoi(tmp);
	if(status != 0 )
	{
	    dbgmsg("load user account list error status=%d, code=%d", status, code);
	    goto ret;
	}
	
	fw = fopen(cache_file, "w+b");
	if(fw)
	{
	    fwrite(buf, strlen(buf), 1, fw);
	    fclose(fw);
    }
        
ret:    
    xml_free_doc(doc);
    if(tmp){
       free(tmp);   
    }
    if(buf){
       free(buf);   
    }
    
}

static void* mysql_cache_loadUserAccountList(void *arg){
    pid_t cache_pid;
    char title[50] = {0};
    while(1)
    {
        cache_pid = fork();
        if(0 == cache_pid) 
		{
		    sprintf(title, "rdsproxy-%s-cache", sys_conf.control.type);
    
            pid_change_title(rds_proxy_argc, rds_proxy_argv, title);
		    mysql_cache_process_load();
		    exit(0);
		}
        sleep(60*3);
    }
	dbgmsg("load thread exit");
	return (void*)0;
}

static void* mysql_cache_loadUserAccountList_bak(void *arg){ 

    char *tmp = NULL;
    char *buf = NULL;
    char sign[128] = {'\0'};
    char param[1024] = {'\0'};
    int bufsize = 1024*1024*10;
    
    char *type = sys_conf.control.type;
    char *url = sys_conf.control.url;
    char *access_id = sys_conf.control.access_id;
    char *access_key = sys_conf.control.access_key;
    char *zone_ids = sys_conf.control.zone_ids;
    char *cache_file = sys_conf.control.cache_file;
    
    FILE *fw = NULL;
    xmlDocPtr doc;
    int status = 0, code = 0;
    
    while(1)
    {
        tmp = malloc(bufsize); 
        buf = malloc(bufsize); 
        if(tmp == NULL || buf == NULL)
        {
            dbgmsg("can not malloc mem.");
            goto ret;
        }
        time_t rawtime;
        time ( &rawtime );
        
        memset(tmp, 0, bufsize);
        memset(sign, 0, sizeof(sign));
        memset(buf, 0, bufsize);
        memset(param, 0, sizeof(param));
        if(strlen(cache_file) < 1)
        {
            goto ret;
        }
        
        sprintf(tmp, "%lu%s%s", rawtime, access_id, access_key);
        
        mysql_cache_get_md5(sign, tmp, 1);
        
        sprintf(param, "format=xml&timestamp=%lu&access_id=%s&sign=%s&type=%s&zoneIds=%s", rawtime, access_id, sign, type, zone_ids);
        
        sprintf(tmp, "curl --data \"%s\" %s%s", param, url, "/instance/userdblist");

        mysql_cache_do_request(buf, bufsize, tmp);
        //dbgmsg("buf:%s",buf);
        mysql_cache_getDecrypt(tmp, buf, access_key);
        //dbgmsg("tmp:%s",tmp);
        if(xml_parse_memory(tmp, strlen(tmp), &doc))
        {
    	    dbgmsg("load user account list. parser xml error.");
    	    goto ret;
    	}
        xml_get_str_by_path_key2(doc, "/res/status", tmp, bufsize, "-1");
    	status = atoi(tmp);
    	xml_get_str_by_path_key2(doc, "/res/code", tmp, bufsize, "-1");
    	code = atoi(tmp);
    	if(status != 0 )
    	{
    	    dbgmsg("load user account list error status=%d, code=%d", status, code);
    	    goto ret;
    	}
    	
    	fw = fopen(cache_file, "w+b");
    	if(fw)
    	{
    	    fwrite(buf, strlen(buf), 1, fw);
    	    fclose(fw);
        }
        
ret:    
        xml_free_doc(doc);
        if(tmp){
           free(tmp);   
        }
        if(buf){
           free(buf);   
        }
        //
        sleep(60*3);
	}
	dbgmsg("load thread exit");
	return (void*)0;
}

int start_mysql_cache_thread(){
    if(strlen(sys_conf.control.cache_file) > 0){
        char ss[1];  
              
        if(launch_thread(mysql_cache_loadUserAccountList, &ss, 0, 1024 * 32, NULL)){
            dbgmsg("start mysql cache thread error.");
            return -1;
        }
        
    }
    return 0;
}


int mysql_cache_get_connect_host(char *username, server_info *serverinfo){
    //char tmp[1024*1024*500] = {'\0'};
    char *tmp = NULL;
    char *access_key = sys_conf.control.access_key;
    char *cache_file = sys_conf.control.cache_file;
    int bufsize = 1024*1024*10;
    
    FILE *fr = NULL;    
    xmlDocPtr doc;
    xmlNodePtr	cur = NULL;
	xmlNodePtr	next = NULL;
    int status = 0, code = 0, n = 0, result = 0;
    
    tmp = malloc(bufsize); 
    if(tmp == NULL){
        dbgmsg("can not malloc mem.");
        goto ret;
    }
    if(username == NULL || strlen(username) < 1)
    {
        goto ret;
    }
    if(strlen(sys_conf.control.cache_file) < 1)
    {
        goto ret;
    }
    
    fr = fopen(cache_file, "rb");
    if(fr == NULL)
    {
       goto ret;  
    }
    fread(tmp, 1, bufsize, fr);
    
    mysql_cache_getDecrypt(tmp, tmp, access_key);
    	
    if(xml_parse_memory(tmp, strlen(tmp), &doc))
    {
	    dbgmsg("load user account list. parser xml error.");
	    goto ret;
	}
    xml_get_str_by_path_key2(doc, "/res/status", tmp, bufsize, "-1");
	status = atoi(tmp);
	xml_get_str_by_path_key2(doc, "/res/code", tmp, bufsize, "-1");
	code = atoi(tmp);
	if(status != 0 )
	{
	    dbgmsg("load user account list error status=%d, code=%d", status, code);
	    goto ret;
	}
    
    if (-1 != xml_get_node_list_by_path_key(doc, "/res/message/item", &cur))
    {
        n = 0;
		while (NULL != cur)
		{
			//syslog server ip
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/username", tmp, bufsize);
			if (0 == result)
			{
			    if(strcmp(tmp, username) != 0)
			    {
			        goto nextitem;
			    }
			    memcpy(serverinfo->username, tmp, strlen(tmp));
			} 
			else 
			{
			    goto nextitem;
			}
			
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/host", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->host, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/port", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->port, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/password", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->password, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/instanceName", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->instance_name, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/dbname", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->dbname, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/audit", tmp, bufsize);
			if (0 == result)
			{
			    memcpy(serverinfo->audit, tmp, strlen(tmp));
	            memset(tmp, 0, 20);
			}
			xml_free_doc(doc);
			if(tmp){
               free(tmp);   
            }
            return 0;
nextitem:			
			if (0 == xml_get_brother_node_by_value(cur, "item", &next))
			{
				cur = next;
				continue;
			}
			else
			{
				break;
			}
		}
    }
ret:
    xml_free_doc(doc);
    if(tmp){
       free(tmp);   
    }
    return -1;    
}


