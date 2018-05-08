#ifndef _LIBXML2_DERIVE_HEAD_
#define _LIBXML2_DERIVE_HEAD_
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

int xml_parse_memory(const char *xml, int len, xmlDocPtr *pdoc);
int xml_parse_file(const char * doc_name, char *charset, xmlDocPtr * pdoc);
void xml_free_doc(xmlDocPtr doc);
int xml_create_file(const char *file_name, char *root_name, char *ver_num, char *charset, xmlDocPtr *doc_ptr);
int xml_get_value_by_path_key(xmlDocPtr doc, char * path_key, char * value, int value_length);
int xml_set_value_by_path_key(char * docname, xmlDocPtr doc, char * path_key, char * value);
int xml_write_value(char *filename, char *path, char *charset, char *value);
int xml_add_value(char *filename, char *path, char *charset, char *value);


void xml_get_str_by_path_key2(xmlDocPtr doc, char *path_key, char *value, int value_length, const char *default_val);
int32_t xml_get_int_by_path_key2(xmlDocPtr doc, char *path_key, int default_val);

int set_profile_path(const char *path, char *charset);
const char *set_profile_main_key(const char *key);
void get_profile_str(const char *entry, char *val, int val_len, const char *default_val);
int32_t get_profile_int(const char * entry, const int default_val);
int32_t set_profile_str(const char *entry, char *val);
int32_t set_profile_int(const char *entry, int val);

int xml_get_one_node_by_path_key(xmlDocPtr doc, char * path_key, xmlNodePtr *node);
int xml_get_node_list_by_path_key(xmlDocPtr doc, char * path_key, xmlNodePtr *node);
int xml_set_node_by_path_key(char * docname, xmlDocPtr doc, char * path_key, xmlNodePtr node);
int xml_set_node_append_by_path_key(char * docname, xmlDocPtr doc, char * path_key, xmlNodePtr node);
int xml_get_brother_node(xmlNodePtr node, xmlNodePtr *brother_node);
int xml_get_brother_node_by_value(xmlNodePtr node, char *value, xmlNodePtr *brother_node);
int xml_get_value_by_path_key_from_node(xmlDocPtr doc, xmlNodePtr node, char * path_key, char * value, int value_length);
int xml_get_group_values_by_path_key(xmlDocPtr doc, char * path_key, char *value, int *row_num, int value_length);
int xml_set_group_values_by_path_key(char * docname, xmlDocPtr doc, char * path_key, char *value, int row_num, int value_length);

int xml_set_value_by_path_key_from_node(char * docname, xmlDocPtr doc, xmlNodePtr node, char * path_key, char * value);
#endif
