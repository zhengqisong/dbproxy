#ifndef _G_LIST_H
#define _G_LIST_H

typedef struct _g_node
{
	char *buf;
	int len;
} g_node;

typedef struct _g_list
{
	int capacity;
	int size;
	struct _g_node *node;
} g_list;

int g_list_init(g_list *list, int capacity);
int g_list_free(g_list *list);
int g_list_size(g_list *list);
char* g_list_get(g_list *list, int index, int *len);
int g_list_add(g_list *list, char* buf, int len);
char* g_list_remove(g_list *list, int index, int *len);
int g_list_remove_all(g_list *list);

#endif
