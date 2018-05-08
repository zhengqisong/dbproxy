#include "proto_parser_conf.h"

parser_proto parser_protos[PROTO_NUM] =
{
	{0, "null", NULL},
	{2, "mysql", mysql_proc},
	{9, "redis", redis_proc},	
};

parser_proto* get_parser_proto_by_raw(char* protocol, char* raw, int len)
{
    //当前只代理mysql
	//unsigned int proto_id = 2;
	int i = 0;
	for(;i < PROTO_NUM; i++)
	{
		//if(parser_protos[i].proto_id == proto_id)
		//{
		//	return &parser_protos[i];
		//}
		if(strcmp(protocol, parser_protos[i].proto_name)==0)
		{
		    return &parser_protos[i];
		}
	}
}
