#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>
#include <signal.h>
#include <pthread.h>
#include <sys/stat.h>
//#include "libxml2_derive.h"
#include "g_xml.h"
#include "g_micro.h"
#include "misc.h"
#include "g_sysv.h"
#include "trace.h"
#include "g_string.h"

int MY_TRACE_ID = TRACE_ID_1;

static int  s_MAX_FILE_LEN = 2 * 1024 * 1024;
static char s_trace_filename[MAX_PATH] = {0};
static char s_trace_filter[MAX_PATH] = {0};
static char s_trace_filter_exclude[MAX_PATH] = {0};
static int  s_trace_ids = -1;
static int  s_trace_ids_exclude = 0;
static int  s_short_prefix = 0;
static int  s_print_tty = 0;
static FILE *f = NULL;

static int32_t shrink_file(){
	FILE *fr = NULL;
	FILE *fw = NULL;
	
	fr = fopen(s_trace_filename, "rb");
	fw = fopen(s_trace_filename, "r+b");
	if(fr && fw){
		char buff[128];
		int rc = 0;
				
		fseek(fr, s_MAX_FILE_LEN / 2, SEEK_SET);
		while(0 < (rc = fread(buff, 1, sizeof(buff), fr))) {
			fwrite(buff, rc, 1, fw);
		}

		ftruncate(fileno(fw), ftell(fw));
	}

	if(fr) fclose(fr);
	if(fw) fclose(fw);

	return 0;
}

static int is_match_filter(const char *fmt, char *p){
	for(; *p; p += strlen(p) + 1){
		if(strstr(fmt, p)) return -1;
	}
	return 0;
}

#include <fcntl.h>
static void _errmsg(int trace_id, long status, int err, 
		const char *code_file, const int code_line,
		const char *fmt, va_list ap){
	static time_t last_time = 0;
	int old_errno = errno;

	if (trace_id != -1) {
		if (!(s_trace_ids & trace_id)) {
			goto end;
		}
	}
	if (s_trace_ids_exclude & trace_id) {
		goto end;
	}
	if ((*s_trace_filter) && (!is_match_filter(fmt, s_trace_filter))) {
		goto end;
	}
	if ((*s_trace_filter_exclude) && (is_match_filter(fmt, s_trace_filter_exclude))) {
		goto end;
	}
	if (!s_trace_filename[0]) {
		strcpy(s_trace_filename, get_exefpath());
		strcat(s_trace_filename, ".log");
	}

	if ((NULL == f) || (1 < (time(NULL) - last_time))) {
		last_time = time(NULL);

		if (f) {
			struct stat stat;
			if (0 == fstat(fileno(f), &stat)) {
				if (0 == stat.st_nlink) {
					fclose(f);
					f = NULL;
				}
			}
		}
		if (NULL == f) {
			f = fopen(s_trace_filename, "a");
		}
	}

	if (f || s_print_tty) {
		static const char *old_code_file = NULL;
		static int old_code_line = -1;
		static int repeat_cnt = 0;
		char prefix[64];
		char loc_time[24];
		time_t t = time(NULL);

		if ((code_line == old_code_line) && (code_file == old_code_file)
			       	&& (repeat_cnt < 100)) {
			repeat_cnt++;
		} else if (0 < repeat_cnt) {
			if (f) {
				fprintf(f, "\n--repeat %d times as above one(from the same code line,"
						" but maybe different contents--\n", repeat_cnt);
			}
			old_code_file = code_file;
			old_code_line = code_line;
			repeat_cnt = 0;
		}

		if (s_short_prefix) {
			strftime(loc_time, sizeof(loc_time), "%H:%M:%S", localtime(&t));
			snprintf(prefix, sizeof(prefix), "%s %s", 
					(-1 == trace_id)? "FAT" : "DBG", loc_time);
		} else {
			strftime(loc_time, sizeof(loc_time), "%Y-%m-%d %H:%M:%S", localtime(&t));
			snprintf(prefix, sizeof(prefix), "%s PID:%u(%x) %s", (-1 == trace_id)? "FAT" : "DBG", 
					(unsigned int)getpid(),(unsigned int )pthread_self(),  loc_time);
		}

		if (f && (0 > fprintf(f, "%s ", prefix))) {
			printf("error in write log file. msg=%s, errno=%d(%s)\n", fmt, errno, strerror(errno));
			fclose(f);
			f = NULL;
			goto end;
		}
		if (s_print_tty) {
			fprintf(stdout, "%s ", prefix);
		}
		if (f) {
			vfprintf(f, fmt, ap);
		}
		if(s_print_tty) {
			vfprintf(stdout, fmt, ap);
		}

		if(err) {
		    if (f) {
				fprintf(f, "[%s (%d)]", strerror(err), err);
			}
			if(s_print_tty) {
				fprintf(stdout, "[%s (%d)]", strerror(err), err);
			}
		}
		if(code_file) {
		       	if (f) {
				fprintf(f, "[%s(%d)]", code_file, code_line);
			}
			if (s_print_tty) {
			//	fprintf(stdout, "[%s(%d)]", code_file, code_line);
			}
		}
		
		if (f) {
			fprintf(f, "\n");
		}
		if (s_print_tty) {
			fprintf(stdout, "\n");
		}

		if(f && (ftell(f) > s_MAX_FILE_LEN)) {
			shrink_file();
		}
		if (f) {
			fflush(f);
		}
	}
end:
	//no relationship with err
	if (-1 == status) EXIT(status);
	else if(1 == status) THREAD_EXIT(status);

	errno = old_errno;
}

void errmsg_ex(int trace_id, long status, int err, 
		const char *code_file, const int code_line,
		const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	_errmsg(trace_id, status, err, code_file, code_line, fmt, ap);
	va_end(ap);
}

void fatal_quit_f(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	_errmsg(-1, -1, errno, NULL, 0, fmt, ap);
	va_end(ap);
}

void fatal_quit_thread_f(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	_errmsg(-1, 1, errno, NULL, 0, fmt, ap);
	va_end(ap);
}

void fatalmsg_f(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	_errmsg(-1, 0, errno, NULL, 0, fmt, ap);
	va_end(ap);
}

void dbgmsg_f(const char *fmt, ...){
	va_list ap;
	va_start(ap, fmt);
	_errmsg(-1, 0, 0, NULL, 0, fmt, ap);
	va_end(ap);
}

void sa_handler_SIGHUP(int signo){
	int old_errno = errno;
	init_trace_ex(NULL, NULL);//not reentrant function
	errno = old_errno;
}

int init_trace_ex(const char *profile_path, const char *main_key){

	static char old_profile_path[MAX_PATH] = {0};
	static char old_main_key[MAX_PATH] = {0};
	xmlDocPtr doc;
	char *p = NULL;
	char xml_path[MAX_PATH];
	char temp1[MAX_PATH];
	char temp2[MAX_PATH];
	
	if(profile_path) {
		strncpy(old_profile_path, profile_path, sizeof(old_profile_path));
	}else {
		profile_path = old_profile_path;
	}
	
	if(main_key) {
		strncpy(old_main_key, main_key, sizeof(old_main_key));
	}else {
		main_key = old_main_key;
	}

	if(xml_parse_file(profile_path, "utf-8", &doc) != 0) {
		printf("init_trace_ex failed %s[%s]\n", profile_path, main_key);
		return -1;
	}

	attach_dir2(xml_path, sizeof(xml_path), main_key, "log_path", NULL);
	xml_get_str_by_path_key2(doc, xml_path, temp1, sizeof(temp1), "./");
	if(s_trace_filename[0]) {
		printf("Old filename:%s, new filename:%s", s_trace_filename, temp1);
	}
	strcpy(s_trace_filename, temp1);

	attach_dir2(xml_path, sizeof(xml_path), main_key, "log_filename", NULL);
	strcpy(temp2, get_exename());
	strcat(temp2, ".log");
	xml_get_str_by_path_key2(doc, xml_path, temp1, sizeof(temp1), temp2);
	attach_dir(s_trace_filename, sizeof(s_trace_filename), temp1, NULL);

	attach_dir2(xml_path, sizeof(xml_path), main_key, "max_file_size", NULL);
	s_MAX_FILE_LEN = xml_get_int_by_path_key2(doc, xml_path, 2 * 1024 * 1024);

	attach_dir2(xml_path, sizeof(xml_path), main_key, "short_prefix", NULL);
	s_short_prefix = xml_get_int_by_path_key2(doc, xml_path, 0);

	attach_dir2(xml_path, sizeof(xml_path), main_key, "print_tty", NULL);
	s_print_tty = xml_get_int_by_path_key2(doc, xml_path, 0);

	attach_dir2(xml_path, sizeof(xml_path), main_key, "trace_ids", NULL);
	xml_get_str_by_path_key2(doc, xml_path, temp1, sizeof(temp1), "");
	if(!temp1[0]) strcpy(temp1, "0");
	temp1[strlen(temp1) + 1] = 0;//end with two 0
	for(p = temp1; *p; p++) {
		if(',' == *p) *p = 0;
	}
	s_trace_ids = 0;
	for(p = temp1; *p; p += strlen(p) + 1){
		s_trace_ids |= atoi(p);
	}
	
	attach_dir2(xml_path, sizeof(xml_path), main_key, "trace_ids_exclude", NULL);
	xml_get_str_by_path_key2(doc, xml_path, temp1, sizeof(temp1), "");
	temp1[strlen(temp1) + 1] = 0;//end with two 0
	for(p = temp1; *p; p++){
		if(',' == *p) *p = 0;
	}
	s_trace_ids_exclude = 0;
	for(p = temp1; *p; p += strlen(p) + 1) {
		s_trace_ids_exclude |= atoi(p);
	}

	attach_dir2(xml_path, sizeof(xml_path), main_key, "trace_filter", NULL);
	xml_get_str_by_path_key2(doc, xml_path, s_trace_filter, sizeof(s_trace_filter) - 2, "");
	s_trace_filter[strlen(s_trace_filter) + 1] = 0;//end with two 0
	for(p = s_trace_filter; *p; p++){
		if(',' == *p) *p = 0;
	}

	attach_dir2(xml_path, sizeof(xml_path), main_key, "trace_filter_exclude", NULL);
	xml_get_str_by_path_key2(doc, xml_path, s_trace_filter_exclude, sizeof(s_trace_filter_exclude) - 2, "");
	s_trace_filter_exclude[strlen(s_trace_filter_exclude) + 1] = 0;//end with two 0
	for(p = s_trace_filter_exclude; *p; p++) {
		if(',' == *p) *p = 0;
	}

	xml_free_doc(doc);
	signal(SIGHUP, sa_handler_SIGHUP);

	//if exists, not change file
	if(f){
		//printf("Change to new log file:%s, no=%d", s_trace_filename, f->_fileno);
		fclose(f);

		f = fopen(s_trace_filename, "a");
		//printf("Change to new log file:%s, no=%d", s_trace_filename, f->_fileno);
	}else{
	    //printf("Change to new log file:%s", s_trace_filename);
		f = fopen(s_trace_filename, "a");
	}

	return 0;
}

void mem_dump(u_int8_t *ucp, int32_t len)
{
	_mem_dump(ucp, len, 1, "");
}

/*
   memory data dump function
param1: memory pointer
param2: length
param3: 0, 1, show summary only(default=TRUE)
param4: prefix-info for every line if memory data(use for filter log)
by pyw[peng_yuwei@venus.com]
 */
void _mem_dump(u_int8_t *ucp, int32_t len, int32_t summary, u_int8_t *info)
{
	if (0 == s_trace_ids) {
		return;
	}

	u_int32_t i = 0;
	int8_t buf[160] = {0};
	int8_t line[255] = {0};
	int8_t text[64] = {0}; 	/* readable memory text*/
	u_int32_t n = 0; 		/* console cursor location*/
	u_int32_t p = 0; 		/* memory pointer*/
	int32_t is_newline = 1;

	if (NULL == ucp) {
		return;
	}

	if (len > 0x4000) {
		dbgmsg("[MEM]mem_dump too long:%d", len);
		return;
	}

	for (i = 0; i < len; i++) {
		if (is_newline) {
			line[0] = '\0';
			sprintf(buf, "%s%04X: ", info ? (char *)info : "",	i);
			v_strncat(line, 160, buf);
			is_newline = 0;
		}

		if (ucp[i] >= 32 && ucp[i] <= 126) {// readable
			sprintf(buf, "%02X ", ucp[i]);
			v_strncat(line, 160, buf);
			text[p] = ucp[i];
		} else if (ucp[i] == 0 || ucp[i] == '%'){
			sprintf(buf, "%02X ", ucp[i]);
			v_strncat(line, 160, buf);
			text[p] = '.';
		} else {
			sprintf(buf, "%02X ", ucp[i]);
			v_strncat(line, 160, buf);
			text[p] = '.';
		}
		n += 3;
		p++;
		if (i % 16 == 15) { // newline
			sprintf(buf, " %s", text);
			v_strncat(line, 160, buf);
			dbgmsg("%s", line);

			n = 0;
			p = 0;
			memset(text, 0, sizeof(text));	
			is_newline = 1;
			//
			if (summary && len > 0x100 && i >= 0x50 && i < len - 0x50) {
				sprintf(line ,
						"%s....:                      (pass %X)\n" ,
						info ? (char *)info : "", (len - (len % 16)) - 0x50 - 1 - i);
				dbgmsg("%s", line);
				i = (len - (len % 16)) - 0x50 - 1;
			}
			//
		}
		else if (i % 8 == 7) { // format display
			v_strncat(line, 160, " ");
			n++;
			text[p++] = ' ';
		}
	}
	if (n > 0 && n < 48) { // complete format
		for (p = n; p <= 48; p++) {
			v_strncat(line, 160, " ");
		}
		sprintf(buf, " %s", text);
		v_strncat(line, 160, buf);
		dbgmsg("%s", line);
	}
}

void enable_dbgmsg()
{
	s_trace_ids = -1;
}
