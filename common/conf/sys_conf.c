#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "sys_conf.h"
#include "trace.h"
#include "g_xml.h"

int sys_conf_init(char *config_file, SYS_CONF *sys_conf)
{
	xmlDocPtr doc;
	xmlNodePtr	cur = NULL;
	xmlNodePtr	next = NULL;
	
	char tmp[128] = {0};
	int result = 0;
	int n = 0;
    memset(sys_conf, 0, sizeof(SYS_CONF));
	//load xml
	if(xml_parse_file(config_file, "utf-8", &doc))
	{
	    fatal_quit_f("sys_conf_init is error........");
		return -1;
	}
	
	//control
	xml_get_str_by_path_key2(doc, "/proxy/control/type", tmp, sizeof(tmp), "");
	memcpy(sys_conf->control.type, tmp, strlen(tmp));
	xml_get_str_by_path_key2(doc, "/proxy/control/protocol", tmp, sizeof(tmp), "mysql");
	memcpy(sys_conf->control.protocol, tmp, strlen(tmp));
	
	if(strcmp(sys_conf->control.type, "synchro") != 0)
	{
    	xml_get_str_by_path_key2(doc, "/proxy/control/url", tmp, sizeof(tmp), "");
    	memcpy(sys_conf->control.url, tmp, strlen(tmp));
    	
    	xml_get_str_by_path_key2(doc, "/proxy/control/access_id", tmp, sizeof(tmp), "");
    	memcpy(sys_conf->control.access_id, tmp, strlen(tmp));
    	
    	xml_get_str_by_path_key2(doc, "/proxy/control/access_key", tmp, sizeof(tmp), "");
    	memcpy(sys_conf->control.access_key, tmp, strlen(tmp));
    	xml_get_str_by_path_key2(doc, "/proxy/control/zone_ids", tmp, sizeof(tmp), "");
    	memcpy(sys_conf->control.zone_ids, tmp, strlen(tmp));    	
    	xml_get_str_by_path_key2(doc, "/proxy/control/cache_file", tmp, sizeof(tmp), "");
    	memcpy(sys_conf->control.cache_file, tmp, strlen(tmp));    	
    }
    xml_get_str_by_path_key2(doc, "/proxy/control/audit", tmp, sizeof(tmp), "");    
	memcpy(sys_conf->control.audit, tmp, strlen(tmp));
	
    
	//event server
//	xml_get_str_by_path_key2(doc, "/proxy/event/server/ip", tmp, sizeof(tmp), "");
//	
//	memcpy(sys_conf->event.server_ip,tmp,strlen(tmp));
//	xml_get_str_by_path_key2(doc, "/proxy/event/server/port", tmp, sizeof(tmp), "30061");
//	sys_conf->event.server_port = atoi(tmp);
	//event syslog
	if (-1 != xml_get_node_list_by_path_key(doc, "/proxy/event/syslog/item", &cur))
	{
		n = 0;
		while (NULL != cur)
		{
			//syslog server ip
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/ip", tmp, sizeof(tmp));
			if (0 == result)
			{
				memcpy(sys_conf->event.syslog_server[n].ip, tmp, sizeof(sys_conf->event.syslog_server[n].ip));
				
			}
			result = xml_get_value_by_path_key_from_node(doc, cur, "/item/port", tmp, sizeof(tmp));
			if (0 == result)
			{
				sys_conf->event.syslog_server[n].port = atoi(tmp);
			}
            n++;
			if(n >= 4)
			{
				break;
			}
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
	xml_free_doc(doc);
}