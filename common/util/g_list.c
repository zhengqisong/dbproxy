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
#include "trace.h"
#include "g_list.h"


int g_list_init(g_list* list, int capacity)
{
	int ret = 0;
	char *p;
	int len = capacity * sizeof(struct _g_node *);
	p = (char*)malloc(len);

	if(p != 0)
	{
		//memset(p, 0, len);
		list->capacity = capacity;
		list->node = (g_node*)p;
		list->size = 0;
	}
	else
	{
		ret = -1;
	}
	return ret;
}

int g_list_free(g_list *list)
{
	g_node *p, *end;
	if(list){
		p = list->node;
		end = p + list->size;
		while(p < end)
		{
			free(p->buf);
			p++;
		}
		free(list->node);
	}

	return 0;
}

int g_list_size(g_list *list)
{
	return list->size;
}

char* g_list_get(g_list* list, int index, int *len)
{
	g_node *p;
	if(list->size <=  index)
	{
		return 0;
	}

	p = list->node;
	p += index;
	*len = p->len;
	return p->buf;
}

int g_list_add(g_list* list, char* buf, int len)
{
	int ret = 0;
	g_node *p;

	if(list->size >= list->capacity)
	{
		ret = -1;
		goto end;
	}

	p = list->node;

    p += list->size;

    p->buf = buf;
    p->len = len;
    list->size += 1;
end:
   return ret;
}

char* g_list_remove(g_list* list, int index, int *len)
{
	g_node *p;
	char *b = 0;
	int n = 0;

	if(list->size <=  index)
	{
		return 0;
	}
	p = list->node;
	p += index;

	b = p->buf;
	*len = p->len;

	n = (list->size - index - 1) * sizeof(struct _g_node *);
	if(n > 0)
	{
		memcpy(p, p + 1, n);
	}
	list->size -= 1;

	return b;
}

int g_list_remove_all(g_list* list)
{
	list->size = 0;
}

