#ifndef _MISC_H
#define _MISC_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>

struct split_var {
	int8_t *addr;
	int32_t maxlen;
};

typedef int(*split_fp)(char *str, void *addr, int maxlen);

int str_bin2asc(unsigned char *bin_str, int bs_len, char *asc_str, int as_maxlen);

char* attach_dir(char *dir, int buff_len, const char *subdir1, const char *subdir2);
char* attach_dir2(char *dir, int buff_len, const char *maindir, const char *subdir1, const char *subdir2);

int sf_s2i(char *str, void *addr, int maxlen);
int sf_scp(char *str, void *addr, int maxlen);
int sf_s2ul(char *str, void *addr, int maxlen);
int sf_s2us(char *str, void *addr, int maxlen);
int str_split(char *orig_str, int delim, split_fp func, struct split_var split_var_table[], int split_var_max);
int32_t str_split_escape(int8_t *src, int8_t delim, 
		struct split_var split_var_table[], int32_t split_var_max);
int getvarbydeli(char *begin,char *end,char *fdeli,int flen,char *ldeli,int llen,char *var,int maxvarlen);
void print_var(char *varname, char *var, int len);
//void get_cmd(Packet_t *packet, char **cmd, int *pcmd_len, char **cmd_para, int *pcmd_para_len);

void print_buf(unsigned char *buf, int nbuf, unsigned int column);
void print_buf2(unsigned char *buf, int nbuf, unsigned int column);
void fprint_buf2(char *path, unsigned char *buf, int nbuf, unsigned int column);

char *new_strchr(char *string, char *delim);
char *mem_find(char *mem_mother, char *mem_son, int mother_len, int son_len);

char *search_sense_char(char *begin, int nsearch, int step);
char *search_nosense_char(char *begin, int nsearch, int step);
//int clean_string_blank(int8_t *str, int32_t len);
int convert(char *fromcode, char *tocode, char *inbuf, int in_len, 
			char *outbuf, int *out_len, int out_total_len);

extern inline void rest(int);

u_int64_t get_cur_microseconds(void);
u_int32_t get_random();
u_int64_t create_session_id(u_int32_t engine_ip, u_int32_t now, u_int32_t ssn_cycle);
u_int64_t create_event_id(u_int32_t engine_ip, u_int32_t now, u_int32_t event_cycle);

//cyt add function to caculate lines numner of a file
int32_t	get_file_linenum(int8_t *filename);

//cyt add function to get ip string from nic_name(like eth0, eth1)
int32_t get_ip_by_dev(int8_t *ifname,int8_t *ip_str, int32_t len, int32_t *ip_int);	
int32_t get_mask_by_dev(int8_t *ifname, int8_t *mask_str, int32_t len, int32_t *mask_int);
int32_t get_dev_ip_by_file(int8_t *ifname, int8_t *ip_str, int32_t len, int32_t *ip_int);

int32_t mask_str2num(int8_t *netmask);
int32_t mask_num2str(int32_t mask_num, int8_t *netmask, int32_t netmask_len);
/* 
 * unsigned int comparison
 */
#define UINT_GT(x,y)     ((y) - (x) > 0x7fffffff)
#define UINT_LT(x,y)     ((x) - (y) > 0x7fffffff)
#define UINT_EQ(x,y)     ((x) == (y))
#define UINT_GE(x,y)     ((x) - (y) < 0x7fffffff)
#define UINT_LE(x,y)     ((y) - (x) < 0x7fffffff)


#ifndef MIN
#define MIN(a, b)	((a) > (b) ? (b) : (a))
#endif //MIN

#ifndef MAX
#define MAX(a, b)	((a) > (b) ? (a) : (b))
#endif //MAX

/* 
 * dump macro for debug begin 
 * Chen Yu - 2003/03/11
 */
#ifdef DEBUG_DUMP
#define DUMP(...) \
	do {\
		printf("DUMP >> ");\
		printf(__VA_ARGS__);\
		printf("\n");\
	} while(0)
#else
#define DUMP(...) \
	do {\
	} while(0)
#endif

#ifdef DEBUG_DUMP_ERR
#define DUMP_ERR(...) \
	do {\
		fprintf(stderr, "** DUMP ERROR <in %s line %d> >> ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__);\
		fprintf(stderr, "\n");\
	} while(0)
#else
#define DUMP_ERR(...) \
	do {\
	} while(0)
#endif

#ifdef DEBUG_DUMP
#define DUMP_R(...) \
	do {\
		printf(__VA_ARGS__);\
	} while(0)
#else
#define DUMP_R(...) \
	do {\
	} while(0)
#endif

/** dump macro for debug end **/

#ifndef offset_of
#define offset_of(str, member)  ((char *)(&((str *)0)->member) - (char *)0)
#endif

#define WARNING(...) \
	do {\
		fprintf(stderr, "** Warning ** <in %s line %d>: ", __FILE__, __LINE__);\
		fprintf(stderr, __VA_ARGS__);\
	} while(0)

#endif // _MISC_H
