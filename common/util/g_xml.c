//#define NDEBUG
#include <assert.h>

#include "g_xml.h"
#include "misc.h"
#include "trace.h"

int xml_parse_file(const char *doc_name, char *charset, xmlDocPtr *pdoc)
{
//	*pdoc = xmlParseFile(doc_name);
	*pdoc = xmlReadFile(doc_name, charset, XML_PARSE_NOBLANKS);
	if (*pdoc == NULL ) {
		fprintf(stderr,"Document not parsed successfully. \n");
		return -1;
	}
	return 0;
}

int xml_parse_memory(const char *xml, int len, xmlDocPtr *pdoc)
{
	*pdoc = xmlParseMemory(xml, len);
	if(*pdoc == NULL)
	{
		fprintf(stderr,"xml not parsed successfully. \n");
		return -1;
	}
	return 0;
}
void xml_free_doc(xmlDocPtr doc)
{
	if (doc != NULL){
		xmlFreeDoc(doc);
		doc = NULL;
	}
}

/*
 * root_name ver_num doc_ptr maybe NULL or not
 * root_name default value is "root"
 * ver_num default value is "1.0"
 * If doc_ptr is NULL, xml file will be created and free document pointer.
 * if doc_ptr is not NULL, create the file 
 *	and it will be assigned the document pointer.
 *	The doc_ptr must be released by caller.
 */
int xml_create_file(const char *file_name, char *root_name, char *ver_num, char *charset, xmlDocPtr *doc_ptr)
{
	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	char rootname[64] = {0};
	char version_number[12] = {0};
	int ret = 0;

	assert(file_name);
	if (NULL == root_name) {
		strncpy(rootname, "root", sizeof(rootname) - 1);
	} else {
		strncpy(rootname, root_name, sizeof(rootname) - 1);
	}
	if (NULL == ver_num) {
		strncpy(version_number, "1.0", sizeof(version_number) - 1);
	}else {
		strncpy(version_number, ver_num, sizeof(version_number) - 1);
	}

	doc = xmlNewDoc(BAD_CAST version_number);
	if (NULL == doc) {
		fprintf(stderr, "Document is not created. doc is NULL\n");
		ret = -1;
		goto err;
	}
	root_node = xmlNewNode(NULL, BAD_CAST rootname);
	if (NULL == root_node) {
		fprintf(stderr, "Document is not created. root_node is NULL\n");
		ret = -1;
		goto err;
	}
	xmlDocSetRootElement(doc, root_node);
	if (-1 == xmlSaveFormatFileEnc(file_name, doc, charset, 1)) {
		fprintf(stderr, "Document is not created. Save xml file failed!\n");
		ret = -1;
		goto err;
	}

	if (NULL != doc_ptr) {
		*doc_ptr = doc;
	}

err:
	if (-1 == ret) {
		if (NULL != doc) {
			xmlFreeDoc(doc);
		}
	}
	return ret;
}

/**
* @doc xmldocPtr
* @value read vlaue
* @value_length read value max lenth
* @path_key /root/keyword/...
* @return 	0 	if success
*	 	-1	fault
*/

int xml_get_value_by_path_key(xmlDocPtr doc, char * path_key, char * value, int value_length)
{
	xmlNodePtr cur, cur_child;
	char *d = "/";
	char *p;
	xmlChar *key;
	char path[1024] = {0};
	int min = 0;

	if (doc == NULL || NULL == path_key || NULL == value){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
	
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			goto err;
		}

		p = strtok(NULL, d);
			
		if (NULL == p) {
			for (cur_child = cur->xmlChildrenNode; cur_child; cur_child = cur_child->next) {
				if (!xmlNodeIsText(cur_child)) {
					break;
				}
			}

			if (cur_child) {
				xmlBufferPtr buff = xmlBufferCreate();
				xmlNodeDump(buff,doc,cur,0,0);
				if (buff->content) {	
					strncpy(value, buff->content, value_length - 1);
					value[value_length - 1] = 0;
				} else {
					value[0] = 0;
				}
				xmlBufferFree(buff);
			}else{
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (key) {	
					strncpy(value, key, value_length - 1);
					value[value_length - 1] = 0;
				} else {
					value[0] = 0;
				}
				xmlFree(key);
			}
			break;
		} else {
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;

}

/**
  * @docname ±£´æµÄÎÄ¼þÃû£¬¿ÉÒÔºÍ´ò¿ªµÄÎÄ¼þ²»Í¬
  * @doc ´ò¿ªµÄÎÄ¼þ¾ä±ú
  * @path_key /root/keyword/...
  * @value  ÐèÒªÐ´ÈëµÄ×Ö·û´®
  * @return       0       ³É¹¦
  *               -1      Ê§°Ü
  *
*/ 
int xml_set_value_by_path_key(char * docname, xmlDocPtr doc, char * path_key, char * value)
{
	xmlNodePtr cur = NULL, cur_parent = NULL;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	int min = 0;

	if (doc == NULL || NULL == path_key || NULL == value){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
		
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);

	p = strtok(path, d);
	while(p)
	{
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				goto err;
			}
		}
		p = strtok(NULL, d);

		if (NULL == p) {
			xmlNodeSetContent(cur, (const xmlChar *)value);
			if (xmlSaveFormatFile (docname, doc, 0) < 0){
				goto err;
			}
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;

}

//����һ���ڵ㣬���ԭ�еĽڵ���ڣ�������
int xml_add_value_by_path_key(char * docname, xmlDocPtr doc, char * path_key, char * value)
{
	xmlNodePtr cur = NULL, cur_parent = NULL;
	char *d = "/";
	char *p = NULL;
	char *p_last = NULL;
	char path[1024] = {0};
	int min = 0;
	xmlChar *key = NULL;

	if (doc == NULL || NULL == path_key || NULL == value){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
		
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);

	p = strtok(path, d);
	while(p)
	{
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				goto err;
			}
		}
		p_last = p;
		p = strtok(NULL, d);

		if (NULL == p) {
			while(cur != NULL) {
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (key) {
					if (xmlStrcmp((const xmlChar *)value, key) == 0) {
						xmlFree(key);
						goto out;
					}
					xmlFree(key);
				}
				cur = cur->next;	
			}
			
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p_last, "")) == NULL){
				goto err;
			}
			xmlNodeSetContent(cur, (const xmlChar *)value);
			if (xmlSaveFormatFile (docname, doc, 0) < 0){
				goto err;
			}
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}
out:
	return 0;

err:
	return -1;

}


int xml_write_value(char *filename, char *path, char *charset, char *value)
{
	if (NULL == filename || NULL == path || NULL == value) {
		goto err;
	}

	xmlDocPtr doc;
	if (-1 == xml_parse_file(filename, charset, &doc)) {
		goto err;
	}

	if (-1 == xml_set_value_by_path_key(filename, doc, path, value)) {
		goto err;
	}

	xml_free_doc(doc);

	return 0;
err:
	return -1;
}


//����һ���ڵ㣬���ԭ�еĽڵ���ڣ�������
int xml_add_value(char *filename, char *path, char *charset, char *value)
{
	if (NULL == filename || NULL == path || NULL == value) {
		goto err;
	}

	xmlDocPtr doc;
	if (-1 == xml_parse_file(filename, charset, &doc)) {
		goto err;
	}

	if (-1 == xml_add_value_by_path_key(filename, doc, path, value)) {
		goto err;
	}

	xml_free_doc(doc);

	return 0;
err:
	return -1;
}


//get one node only, set it's next to null
int xml_get_one_node_by_path_key(xmlDocPtr doc, char * path_key, xmlNodePtr *node)
{
	xmlNodePtr cur;
	xmlNodePtr tempNode;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	int min = 0;

	if (doc == NULL || NULL == path_key || NULL == node){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
	
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			goto err;
		}

		p = strtok(NULL, d);
			
		if (NULL == p) {
			*node = cur;
			cur = cur->next;
			while (cur) {
				tempNode = cur->next;
				xmlUnlinkNode(cur);
				xmlFreeNode(cur);
				cur = tempNode;
			}
			//memory
			(*node)->next = NULL;
			break;
		} else {
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;

}

int xml_get_node_list_by_path_key(xmlDocPtr doc, char * path_key, xmlNodePtr *node)
{
	xmlNodePtr cur;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	int min = 0;

	if (doc == NULL || NULL == path_key || NULL == node){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
	
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			goto err;
		}

		p = strtok(NULL, d);
			
		if (NULL == p) {
			*node = cur;
			break;
		} else {
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;

}

int xml_set_node_by_path_key(char * docname, xmlDocPtr doc, char * path_key, xmlNodePtr node)
{
	xmlNodePtr cur = NULL;
	xmlNodePtr cur_parent = NULL;
	xmlNodePtr delNode = NULL;
	xmlNodePtr cpyNode = NULL;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	int min = 0;
	int ret = 0;

	if (doc == NULL || NULL == path_key || NULL == node || NULL == docname){
		ret = -4;
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		ret = -5;
		goto err;
	}

	cpyNode = xmlCopyNodeList(node);
	if (NULL == cpyNode) {
		goto err;
	}
		
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);

	p = strtok(path, d);
	while(p)
	{
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				ret = -2;
				goto err;
			}
		}
		p = strtok(NULL, d);

		if (NULL == p) {
#if 1
			if (NULL != cur->xmlChildrenNode) {
				delNode = cur->xmlChildrenNode;
				if (NULL != delNode) {
					xmlFreeNodeList(delNode);
					cur->xmlChildrenNode = NULL;
				}
				xmlAddChild(cur, cpyNode);
			} else {
				xmlAddChild(cur, cpyNode);
			}
#endif
			if (xmlSaveFormatFile (docname, doc, 0) < 0){
				ret = -3;
				goto err;
			}
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return ret;
}

int xml_set_node_append_by_path_key(char * docname, xmlDocPtr doc, char * path_key, xmlNodePtr node)
{
	xmlNodePtr cur = NULL;
	xmlNodePtr cpyNode = NULL;
	xmlNodePtr cur_parent = NULL;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	char name[1024] = {0};
	int min = 0;

	if (doc == NULL || NULL == path_key || NULL == node || NULL == docname){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
		
	cpyNode = xmlCopyNodeList(node);
	if (NULL == cpyNode) {
		goto err;
	}
		
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);

	p = strtok(path, d);
	while(p)
	{
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				goto err;
			}
		}
		strcpy(name, p);
		p = strtok(NULL, d);

		if (NULL == p) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)name, "")) == NULL){
				goto err;
			}
			xmlAddChild(cur, cpyNode);
			if (xmlSaveFormatFile (docname, doc, 0) < 0){
				goto err;
			}
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;
}

int xml_get_brother_node(xmlNodePtr node, xmlNodePtr *brother_node)
{
	xmlNodePtr cur;

	if (NULL == node || NULL == brother_node) {
		goto err;
	}

	cur = node->next;
	while (cur) {
		if (!xmlNodeIsText(cur)) {
			*brother_node = cur;
			break;
		}

		cur = cur->next;
	}

	if (NULL == cur) {
		goto err;
	}
	
	return 0;

err:
	return -1;

}

int xml_get_brother_node_by_value(xmlNodePtr node, char *value, xmlNodePtr *brother_node)
{
	xmlNodePtr cur;

	if (NULL == node || NULL == brother_node || NULL == value) {
		goto err;
	}

	cur = node->next;
	while (cur) {
		if (xmlStrcmp((const xmlChar *)value, cur->name) == 0) {
			*brother_node = cur;
			break;
		}

		cur = cur->next;
	}

	if (NULL == cur) {
		goto err;
	}
	
	return 0;

err:
	return -1;
}

int xml_get_value_by_path_key_from_node(xmlDocPtr doc, xmlNodePtr node, char * path_key, char * value, int value_length)
{
	xmlNodePtr cur, cur_child;
	char *d = "/";
	char *p;
	xmlChar *key;
	char path[1024] = {0};
	int min = 0;

	if (node == NULL || NULL == path_key || NULL == value){
		goto err;
	}

	cur = node;

	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	if (0 != xmlStrcmp((const xmlChar *)p, cur->name)) {
		goto err;
	}
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			goto err;
		}

		p = strtok(NULL, d);
			
		if (NULL == p) {
			for (cur_child = cur->xmlChildrenNode; cur_child; cur_child = cur_child->next) {
				if (!xmlNodeIsText(cur_child)) {
					break;
				}
			}

			if (cur_child) {
				xmlBufferPtr buff = xmlBufferCreate();
				xmlNodeDump(buff, doc, cur, 0, 0);
				if (buff->content) {	
					strncpy(value, buff->content, value_length - 1);
					value[value_length - 1] = 0;
				}
				xmlBufferFree(buff);
			}else{
				key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if (key) {	
					strncpy(value, key, value_length - 1);
					value[value_length - 1] = 0;
				} else {
					// add by fix if is a empty node mustbe return \0 such as <path></path>
					value[0] = '\0';
				}
				xmlFree(key);
			}
			break;
		} else {
			cur = cur->xmlChildrenNode;
		}
	}

	return 0;

err:
	return -1;
}

int xml_set_value_by_path_key_from_node(char * docname, xmlDocPtr doc, xmlNodePtr node, 
		char * path_key, char * value)
{
	xmlNodePtr cur, cur_parent = NULL;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	int min = 0;

	if (node == NULL || NULL == path_key || NULL == value){
		goto err;
	}

	cur = node;

	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	if (0 != xmlStrcmp((const xmlChar *)p, cur->name)) {
		goto err;
	}
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				goto err;
			}
		}
		p = strtok(NULL, d);
			
		if (NULL == p) {
				xmlNodeSetContent(cur, (const xmlChar *)value);
				if (xmlSaveFormatFile (docname, doc, 0) < 0){
					goto err;
				}
			break;
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}

	return 0;

err:
	return -1;
}

int xml_get_group_values_by_path_key(xmlDocPtr doc, char * path_key, char *value, int *row_num, int value_length)
{
	xmlNodePtr cur, cur_child;
	char *d = "/";
	char *p;
	xmlChar *key;
	char path[1024] = {0};
	char name[1024] = {0};
	int min = 0;
	int i = 0;

	if (doc == NULL || NULL == path_key || NULL == value || *row_num <= 0){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
	
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);	
	
	p = strtok(path, d);
	
       	while(p)
	{	
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			goto err;
		}

		strcpy(name, p);
		p = strtok(NULL, d);
			
		if (NULL == p) {
			for (i = 0; cur && i < *row_num; i++) {
				for (cur_child = cur->xmlChildrenNode; cur_child; cur_child = cur_child->next) {
					if (!xmlNodeIsText(cur_child)) {
						break;
					}
				}

				if (cur_child) {
					xmlBufferPtr buff = xmlBufferCreate();
					xmlNodeDump(buff,doc,cur,0,0);
					if (buff->content) {	
						strncpy(value + (i * value_length), buff->content, value_length - 1);
						value[i * value_length + value_length - 1] = 0;
					}
					xmlBufferFree(buff);
				}else{
					key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
					if (key) {	
						strncpy(value + (i * value_length), key, value_length - 1);
						value[i * value_length + value_length - 1] = 0;
					}
					xmlFree(key);
				}

				cur = cur->next;
				while (cur && xmlStrcmp((const xmlChar *)name, cur->name)){
					cur = cur->next;
				}

			}

			*row_num = i;
			break;
		} else {
			cur = cur->xmlChildrenNode;
		}
	}

	return 0;
err:
	return -1;

}

int xml_set_group_values_by_path_key(char * docname, xmlDocPtr doc, char * path_key, char *value, int row_num, int value_length)
{
	xmlNodePtr cur = NULL, cur_parent = NULL, next = NULL, tmp = NULL;
	char *d = "/";
	char *p;
	char path[1024] = {0};
	char name[1024] = {0};
	int min = 0;
	int i = 0;

	if (doc == NULL || NULL == path_key || NULL == value || NULL == docname || row_num <= 0){
		goto err;
	}
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL){
		goto err;
	}
		
	min = 1023 > strlen(path_key) ? strlen(path_key) : 1023;
	strncpy(path, path_key, min);

	p = strtok(path, d);
	while(p)
	{
		while (cur != NULL) {
			if (xmlStrcmp((const xmlChar *)p, cur->name) == 0) {
				break;
			}
			cur = cur->next;
		}

		if (NULL == cur) {
			if ((cur = xmlNewChild(cur_parent, NULL, (const xmlChar *)p, "")) == NULL){
				goto err;
			}
		}

		strcpy(name, p);
		p = strtok(NULL, d);

		if (NULL == p) {
			xmlNodeSetContent(cur, (const xmlChar *)(value + i * value_length));
			for (i = 0; i < row_num && 0 != value[i * value_length]; i++) {
				tmp = cur->next;
				if ((next = xmlNewNode(NULL, name)) == NULL){
					goto err;
				}
				next->next = tmp;
				cur->next = next;
				cur = cur->next;

				xmlNodeSetContent(cur, (const xmlChar *)(value + i * value_length));
			}
			if (xmlSaveFormatFile (docname, doc, 0) < 0){
				goto err;
			}
		} else {
			cur_parent = cur;
			cur = cur->xmlChildrenNode;
		}
	}
	return 0;

err:
	return -1;
}

char* strncpy2(char *dest, const char *src, size_t count)
{
	strncpy(dest, src, count);
	if(dest[count - 1]){
		dest[count - 1] = 0;
	}
	return dest;
}

void xml_get_str_by_path_key2(xmlDocPtr doc, char *path_key, char *value, int value_length, const char *default_val)
{
	if(xml_get_value_by_path_key(doc, path_key, value, value_length) || (!value[0])){
		strncpy2(value, default_val, value_length);
	}
}

int32_t xml_get_int_by_path_key2(xmlDocPtr doc, char *path_key, int default_val)
{
	int8_t buf[32];
	if(xml_get_value_by_path_key(doc, path_key, buf, sizeof(buf)) || (!buf[0])){
		return default_val;
	}
	return atoi(buf);
}


static xmlDocPtr app_doc = NULL;
static char old_main_key[256]= {0};
static char main_key[256] = {0};

int set_profile_path(const char *path, char *charset){
	if(app_doc) xml_free_doc(app_doc);
	if (xml_parse_file(path, charset, &app_doc) == 0) return -1;
	return 0;
}

const char *set_profile_main_key(const char *key){
	strcpy(old_main_key, main_key);
	strncpy(main_key, key, sizeof(main_key));
	if(main_key[sizeof(main_key)])
		main_key[sizeof(main_key)] = 0;

	return old_main_key;
}

void get_profile_str(const char *entry, char *val, int val_len, const char *default_val){
	char fpath[sizeof(main_key)];

	attach_dir2(fpath, sizeof(fpath), main_key, entry, NULL);
	xml_get_str_by_path_key2(app_doc, fpath, val, val_len, default_val);
}

int32_t get_profile_int(const char * entry, const int default_val){
	char fpath[sizeof(main_key)];

	attach_dir2(fpath, sizeof(fpath), main_key, entry, NULL);
	return xml_get_int_by_path_key2(app_doc, fpath, default_val);
}

int32_t set_profile_str(const char *entry, char *val){
	return 0;
}

int32_t set_profile_int(const char *entry, int val){
	return 0;
}


#if 0
int main(int argc, char **argv) {
	char *docname;
	char *path;
	xmlDocPtr doc;
	//char buff[30][100] = {0};
	char buff[10000] = {0};
	int count = 30;
	int i = 0;
	if (argc <= 2) {
		printf("Usage: %s docname path\n", argv[0]);
		return(0);
	}
	docname = argv[1];
	path = argv[2];
	
	if (xml_parse_file(docname, "utf-8", &doc) == 0){

		xml_get_value_by_path_key(doc, path, buff,10000);
		printf("%s\n",buff);
	
	//	xml_get_array_by_path_key(doc, path, buff, &count, 100);

		/*
		for (i = 0; i < count, i++){
			printf("%s\n", buff[i]);
		}
		*/

		xml_free_doc(doc);
	}
	return (1);
}
#endif
