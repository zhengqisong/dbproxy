#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <iconv.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include "misc.h"

//for get_ip_from_dev
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "g_string.h"

static unsigned char transtbl_i2a[16] = "0123456789ABCDEF";
/*
 * str_bin2asc
 * example: 0x12345678 --> "12 34 56 78\0"
 * so if want change all binary string of X Bytes, the length of 
 * ascii string must be 3*XBytes
 */
int str_bin2asc(unsigned char *bin_str, int bs_len, char *asc_str, int as_maxlen)
{
	int as_len = 0;
	int real_bs_len = 0;
	int i;
	unsigned char low, high;

	if (!bin_str || !asc_str)
		goto err;

	real_bs_len = as_maxlen/3 > bs_len ? bs_len : as_maxlen/3;

	for (i = 0; i < real_bs_len; i++) {
		high = bin_str[i] >> 4;
		low  = bin_str[i] & 0xf;
	
		asc_str[as_len++] = transtbl_i2a[high];
		asc_str[as_len++] = transtbl_i2a[low];
		asc_str[as_len++] = ' ';
	}
	if (as_len) {
		as_len--;
		asc_str[as_len] = '\0';
	}
	
	return as_len;
err:
	return -1;
}

char* attach_dir(char *dir, int buff_len, const char *subdir1, const char *subdir2){
	int dir_len = strlen(dir);
	const int sub1_len = subdir1? strlen(subdir1) : 0;
	const int sub2_len = subdir2? strlen(subdir2) : 0;

	if(buff_len < dir_len + sub1_len + sub2_len + 3)
		exit(0);

	if(subdir1){
		if(('/' == dir[dir_len - 1]) && ('/' == *subdir1)) dir[dir_len--] = 0;
		if(('/' != dir[dir_len - 1]) && ('/' != *subdir1)) dir[dir_len++] = '/';
		strcpy(dir + dir_len, subdir1);
		dir_len += sub1_len;
	}   

	if(subdir2){
		if(('/' == dir[dir_len - 1]) && ('/' == *subdir2)) dir[dir_len--] = 0;
		if(('/' != dir[dir_len - 1]) && ('/' != *subdir2)) dir[dir_len++] = '/';
		strcpy(dir + dir_len, subdir2);
	}   

	return dir;
}

char* attach_dir2(char *dir, int buff_len, const char *maindir,
		const char *subdir1, const char *subdir2){
	int dir_len = strlen(maindir);
	const int sub1_len = subdir1? strlen(subdir1) : 0;
	const int sub2_len = subdir2? strlen(subdir2) : 0;

	if(buff_len < dir_len + sub1_len + sub2_len + 3)
		exit(0);
	strcpy(dir, maindir);

	if(subdir1){
		if(('/' == dir[dir_len - 1]) && ('/' == *subdir1)) dir[dir_len--] = 0;
		if(('/' != dir[dir_len - 1]) && ('/' != *subdir1)) dir[dir_len++] = '/';
		strcpy(dir + dir_len, subdir1);
		dir_len += sub1_len;
	}

	if(subdir2){
		if(('/' == dir[dir_len - 1]) && ('/' == *subdir2)) dir[dir_len--] = 0;
		if(('/' != dir[dir_len - 1]) && ('/' != *subdir2)) dir[dir_len++] = '/';
		strcpy(dir + dir_len, subdir2);
	}

	return dir;
}

#if 0
int DUMP(const char *format, ...)
{
#ifdef DEBUG_DUMP
	va_list ap;
	int ret;

	va_start(ap, format);
	ret = vprintf(format, ap);
	va_end(ap);

	return ret;
#else
	return 0;
#endif
}


/*
 * -1 failed, >= 0  ok.
 */
inline int get_first_str(char *begin, char *end, char *buff, int bufflen)
{
	int count;


	if (begin >= end) return -1;

	for (count = 0; (begin + count < end && count < bufflen); count++) {
		if (isspace(*(begin + count))) {
			if (count != 0) {
				strncpy(buff, begin, count);
				buff[count] = 0;
			}

			return count;
		}
	}

	if (begin + count == end) {
		
		if (count != 0) {
			strncpy(buff, begin, count);
			buff[count] = 0;
		}

		return count;
	}

	return -1;
}

#endif

#if 0
main(int argc, char **argv)
{
	char asc_str[96];
	char bin_str[32];
	int bin_str_sz;
	int i;
	int len;

	bin_str_sz = sizeof(bin_str);
	for (i = 0; i < bin_str_sz; i++)
	{
		bin_str[i] = i;
	}
	len = str_bin2asc(bin_str, sizeof(bin_str), asc_str, sizeof(asc_str));
	assert(len == strlen(asc_str));
	printf("len(%d) str(%s)\n", len, asc_str);

	return 0;
}
#endif


/*This function get var between two deli
	0: failed
   	>1:successed
	begin: 	databuffer's begin
	end :  	databuffer's end
	fdeli: 	begin's Deli
	flen:  	begin's Deli length
	ldeli:  last's Deli
	llen:   last'Deli length
	var : 	the return var
	varlen:	var's length;

*/   
int getvarbydeli(char *begin,char *end,char *fdeli,int flen,char *ldeli,int llen,
		char *var,int maxvarlen)
{
	int varlen;
	char *index ;
	char *fvar = 0;
	char *lvar = 0;
	/*from begin to end */
	for( index = begin; index < end; index++ ) {
		if(flen == 1) {
			if(*index == *fdeli) {
				fvar = index + 1;
				break;
			} else {
				continue;
			}
		}
	}
	if(!fvar) return 0;

	/*from end to begin */
	index = end-1;
	while( index > begin ) {
		if(llen == 1) {
			if(*index == *ldeli) {
				lvar = index;
				break;
			} else {
				index--;
				continue;
			}

		}	
	}
	if(!lvar) return 0;
	varlen = lvar - fvar;	
	if (varlen >= maxvarlen) {
		return 0;
	}
	strncpy(var,fvar,varlen);
	var[varlen] = '\0';
#ifdef	DEBUG_DUMP
	printf("%s",var);
#endif
	return 1;

}

#if 0
/*
 *
 * CSC for process pop3 and smtp
 * Packet like this:
 * Command + SP + Command paras + CTL/LN
 */ 

void get_cmd(Packet_t *packet, char **cmd, int *pcmd_len, char **cmd_para, int *pcmd_para_len)
{
	char *index, *begin, *end;
	int i = 0;

	*pcmd_len = 0;
	*pcmd_para_len = 0;	
	assert(packet);
	index = begin = packet->tcp_payload;
	end = begin + packet->tcp_payload_len;
	*cmd = begin;
	while(index < end) {
		i++;
		if(*index == 0x20) {
			*cmd_para = index + 1;
			break;
		}
		if(*index == 0x0d) {
			*pcmd_len = i - 1;
			*pcmd_para_len = 0;
			return;
		}
		index++;
	}
	*pcmd_len = i - 1;
	i = 0;
	index++;
	while(index < end) {
		i++;
		if(*index == 0x0d) 
			break;
		index++;	
	}
	*pcmd_para_len = i - 1;
	/*
	DUMP_R("Para cmd len [%d] \n", *pcmd_len);
	if(*pcmd_len > 0) {
		for(i = 0; i < *pcmd_len; i++) {
			DUMP_R("%c", *(cmd + i));
		}
	}
	DUMP_R("\n");	
	DUMP_R("Para cmd param len [%d] \n", *pcmd_para_len);
	if(*pcmd_para_len > 0) {
		for(i = 0; i < *pcmd_para_len; i++) {
			DUMP("%c", *(cmd_para + i));
		}
	}	
	DUMP_R("\n");	
	*/
		
	return;	
}
#endif

void print_var(char *varname, char *var, int len)
{
	int i = 0;
	printf("\n------------------------------\n");
	printf(" %s: \n", varname);
	for(i = 0; i < len; i++) {
		printf("%c", *(var + i));
	}
	printf("\n-------------------------------\n");
}		
	

//
// split funcs
// return: n >= 0, result len in "addr", n < 0, error
// sf_s2i: convert string to int
int sf_s2i(char *str, void *addr, int maxlen)
{
	int n = 0;

	if (!addr || maxlen < 4)
		goto err;

	if (!str)
		*(int *)addr = 0;
	else
		*(int *)addr = atoi(str);

	n = sizeof(int);

	return n;
err:
	return -1;
}

int sf_scp(char *str, void *addr, int maxlen)
{
	int n = 0;

	if (!addr || maxlen < 1)
		goto err;

	if (!str) {
		*(char *)addr = 0;
		goto ret;
	}

	if (strlen(str)+1 > maxlen) {
		*(char *)addr = 0;
		goto ret;
	}

	strcpy(addr, str);
ret:
	return n;
err:
	return -1;
}

//
// split a string to n part, every part will be a param to 'func', and fill the result
// to the address stored in struct split_var 's addr.
// if 'func' == NULL, call string copy(sf_scp) as default func
int str_split(char *orig_str, int delim, split_fp func, struct split_var split_var_table[], int split_var_max)
{
	char *tbuf = NULL;
	char *head = NULL, *tail = NULL;
	int nvar = 0;
	char *p = NULL;
	split_fp callfunc = NULL;

	if (!orig_str || !orig_str[0]) {
		goto err;
	}

	tbuf = malloc(strlen(orig_str) + 1);
	if (!tbuf)
		goto err;
	strcpy(tbuf, orig_str);

	tail = tbuf;
	while (1) {
		if (!tail)
			break;

		head = tail;
		p = strchr(tail, delim);
		if (p) {
			*p = '\0';
			tail = p+1;
		} else
			tail = NULL;

		if (func)
			callfunc = func;
		else
			callfunc = sf_scp; /* default: string copy */

		if (callfunc(head, split_var_table[nvar].addr, split_var_table[nvar].maxlen) < 0) {
			fprintf(stderr, "Error: splited data convert error(%s)\n", head);
			continue;
		}

		nvar++;
		if (nvar == split_var_max)
			break;
	}

	free(tbuf);

	return nvar;
err:
	return -1;
}

#if 0
int main(int argc, char **argv)
{
	int var1, var2;
	struct split_var var_table[5];
	int nsplit;

	var_table[0].addr = &var1;
	var_table[0].maxlen = sizeof(var1);

	var_table[1].addr = &var2;
	var_table[1].maxlen = sizeof(var2);

	nsplit = str_split(argv[1], ',', sf_s2i, var_table, 2);
	printf("nsplit:%d, var1:%d, var2:%d\n", nsplit, var1, var2);

	return 0;
}

#endif

void print_buf(unsigned char *buf, int nbuf, unsigned int column)
{
	int i;
	int col;

	if (column == 0)
		col = 30;
	else
		col = column;

//	printf("PrintBuf:Len:%d\n", nbuf);
	for (i = 0; i < nbuf; i++) {
		printf("%x%x ", buf[i]>>4, buf[i]&0x0f);
		if ((i+1) % col == 0)
			printf("\n");
	}
	printf("\n");

	return;
}

void fprint_buf2(char *path, unsigned char *buf, int nbuf, unsigned int column)
{
	int i, j;
	int col;
	unsigned char ch;
	FILE * fp = NULL;

	if (NULL == (fp = fopen(path, "a+"))) {
		printf("open file :%s failed,errno:%d(%s)\n",
				path, errno, strerror(errno));
		goto err;
	}

	if (column == 0)
		col = 16;
	else
		col = column;

	for (i = 0; i < nbuf; i++) {
		if (i % col == 0){
			fprintf(fp, "0x%.8x  ", (int)(buf + i));
		}

		fprintf(fp, "%x%x ", buf[i] >> 4, buf[i] & 0xf);
		if ((i + 1) % col == 0) {
			for (j = 0; j < col; j++) {
				ch = *(buf + i + j - (col-1));
				fprintf(fp, "%c", isprint (ch) ? ch : '.');
			}
			fprintf(fp, "\n");
		}
	}
	for (j = 0; j < col - i % col; j++)
		fprintf(fp, "   ");
	for (j = 0; j < i % col; j++) {
		ch = *(buf + i + j - i % col);
		fprintf(fp, "%c", isprint (ch) ? ch : '.');
	}
	fprintf (fp, "\n");
	fclose(fp);

err:
	return;
}
void print_buf2(unsigned char *buf, int nbuf, unsigned int column)
{
	int i, j;
	int col;
	unsigned char ch;

	if (column == 0)
		col = 16;
	else
		col = column;

//	printf("PrintBuf:Len:%d\n", nbuf);
	for (i = 0; i < nbuf; i++) {
		printf ("%x%x ", buf[i] >> 4, buf[i] & 0xf);
		if ((i + 1) % col == 0) {
			for (j = 0; j < col; j++) {
				ch = *(buf + i + j - (col-1));
				printf ("%c", isprint (ch) ? ch : '.');
			}
			printf ("\n");
		}
	}
	for (j = 0; j < col - i % col; j++)
		printf ("   ");
	for (j = 0; j < i % col; j++) {
		ch = *(buf + i + j - i % col);
		printf ("%c", isprint (ch) ? ch : '.');
	}
	printf ("\n");

	return;
}


#if 0
int main(int argc, char **argv)
{
	char *addr = NULL;
	int len;

	if (argc == 2)
		len = atoi(argv[1]);
	else
		len = 400;

	addr = malloc(len);

	print_buf(addr, len, 0);

	print_buf2(addr, len, 0);

	free(addr);

	return 0;
}
#endif

//
// new_strchr
// search any char in 'delim' which is first ocurr in 'string'
char *new_strchr(char *string, char *delim)
{
        int str_i;

        if (string == NULL || delim == NULL)
                return NULL;

        for (str_i = 0; str_i < strlen(string); str_i++) {
                char *p;

                p = strchr(delim, string[str_i]);  // find char in delimiters
                if (p != NULL) {
                        break;
                }
        }

        if (str_i == strlen(string)) {
                return NULL;
        } else
                return string+str_i;

}
char *mem_find(char *mem_mother, char *mem_son, int mother_len, int son_len){
	int i;
	if(mem_mother == NULL||mem_son == NULL||mem_son == 0||son_len == 0){
		return NULL;
	}
	for(i=0;i<mother_len-son_len;i++){
		if(memcmp(mem_mother+i,mem_son,son_len) == 0){
			return mem_mother+i;
		}
	}
	return NULL;
}

/* 
 * 找出三个指针中的最小一个非空指针
 * Add by Chen Yu 
 */
unsigned min_p(unsigned long a, unsigned long b, unsigned long c)
{
	unsigned long min;

	min = (a-1)<(b-1)?(a-1):(b-1);
	if (c - 1 < min)
		min = c - 1;
	return min + 1;
}

/* 
 * Add by Chen Yu 
 */
void rest(int sleep_time)
{
#define GPACK_USLEEP
#ifdef GPACK_USLEEP
			usleep(sleep_time);
#else
			sched_yield();
#endif
}

// search_nosense_char
// 寻找无意义的字符，这些字符是指' ', '\t', '\n', '\r'和0字符
// NULL: not found
// step: >=0 = forward search
// 	<0 = backward search
// 
char *search_nosense_char(char *begin, int nsearch, int step)
{
	char *p;
	char *pret = NULL;
	int i;

	p = begin;
	for (i = 0; i < nsearch; i++) {
		if (*p == 0 || *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
			pret = p;
			goto ret;
		}

		if (step >= 0)
			p++;
		else 
			p--;
	}

ret:
	return pret;
}

// search_sense_char
// 寻找有意义的字符，这些字符是指除了' ', '\t', '\n', '\r'和0之外的字符
// NULL: not found
// step: >=0 = forward search
// 	<0 = backward search
// 
char *search_sense_char(char *begin, int nsearch, int step)
{
	char *p;
	char *pret = NULL;
	int i;

	p = begin;
	for (i = 0; i < nsearch; i++) {
		if (*p != 0 && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') {
			pret = p;
			goto ret;
		}

		if (step >= 0)
			p++;
		else 
			p--;
	}

ret:
	return pret;
}
#if 0
int clean_string_blank(int8_t *str, int32_t len)
{
	char *begin = str;
	char *end = str + len - 1;
	char tmp_str[1024] = {0};
	int ret = 0;

	if (len <= 0 || NULL == str) {
		goto out;	
	}	
	while (' ' == *begin && begin < end) {
		begin++;	
	}
	while (' ' == *end && end > begin) {
		end--;
	}

	if (end == begin && *begin == ' ') {
		goto out;
	}

	strncpy(tmp_str, begin, end - begin + 1);
	memset(str, 0x00, len);
	strncpy(str, tmp_str, strlen(tmp_str));

	ret = strlen(str);

out:	
	return ret;
}
#endif

/*
   get current microseconds,20080901
*/
u_int64_t get_cur_microseconds()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (u_int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

u_int32_t get_random()
{
	int fd;
	u_int32_t n = 0;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd > 0) {
		read(fd, &n, sizeof(n));
		close(fd);
	} else {
		n = time(NULL);
	}

	return n;
}

u_int64_t create_session_id(u_int32_t engine_ip, u_int32_t now, u_int32_t ssn_cycle) 
{
	return (((u_int64_t)engine_ip) << 48) + (((u_int64_t)(now & 0xFFFFFF)) << 24)
			+ (u_int64_t)(ssn_cycle & 0xFFFFFF);
}

u_int64_t create_event_id(u_int32_t engine_ip, u_int32_t now, u_int32_t event_cycle) 
{
	return (((u_int64_t)engine_ip) << 48) + (((u_int64_t)(now & 0xFFFFFF)) << 24)
			+ (u_int64_t)(event_cycle & 0xFFFFFF);
}

int32_t	get_file_linenum(int8_t *filename)
{
	int32_t	count = 0;
	FILE 	*fd = NULL;
	int8_t	*msg = NULL;
	int32_t	len = 0;

	if (NULL == filename) {
		goto err;
	}

	fd = fopen(filename, "r");
	if (NULL == fd) {
		goto err;
	}
	while (-1 != getline((char **)&msg, &len, fd)) {
		count++;
	}
	fclose(fd);
	free(msg); //getline mallocs the memory needed, and free it here

err:
	return count;
}


#define ETH_CFG_FILE "/etc/sysconfig/network-scripts/ifcfg-"

int32_t get_dev_ip_by_file(int8_t *ifname, int8_t *ip_str, int32_t len, int32_t *ip_int)
{
	int8_t filename[128] = {0};
	FILE *fd = NULL;
	int8_t buf[256] = {0};
	int8_t *p = NULL;
	int8_t ip[20] = {0};
	int8_t *ip_begin = NULL;

	if (NULL == ifname) {
		goto err;
	}	
	snprintf(filename, sizeof(filename), "%s%s", ETH_CFG_FILE, ifname);
	fd = fopen(filename,"r");
	if (fd == NULL) {
		printf("Can't access %s \n",filename);
		goto err;
	}
	while (0 == feof(fd)) {
		if (NULL == fgets(buf, sizeof(buf), fd)) {
			goto err;
		}
		if (strstr(buf, "IPADDR")) {
			ip_begin = strstr(buf, "=");
			if (NULL == ip_begin || ip_begin - buf == sizeof(buf) - 1 
					||  *(ip_begin + 1) == '\0') {
				goto err;
			}
			p = strchr(buf, '\"');
			if (NULL == p) {
				strncpy(ip, ip_begin + 1, sizeof(ip));
			} else {
				if (p - buf == sizeof(buf) - 1 || *(p + 1) == '\0') {
					goto err;
				} else {
					ip_begin = p + 1;
					p = strchr(ip_begin, '\"');
					if (NULL != p) {
						*p = '\0';
					}
					strncpy(ip, ip_begin, sizeof(ip));
				}
			}
			break;
		}
	}
	if ('\0' != ip[0] && NULL != ip_int) {
		*ip_int = inet_addr(ip);
	}

	if ('\0' != ip[0] && NULL != ip_str) {
		strncpy(ip_str, ip, len);
	}
	fclose(fd);
	fd = NULL;
	return 0;
err:
	if (NULL != fd) {
		fclose(fd);
		fd = NULL;
	}
	return -1;
}

int32_t mask_str2num(int8_t *netmask)
{
	u_int32_t u_net_mask = inet_network(netmask);
	if (u_net_mask == 0) {
		return 0;
	}
	u_int32_t num = 0;
	u_net_mask = ~u_net_mask + 1;
	int i = 0;
	for (i = 1; i < 32; i++) {
		num = 1 << i;
		if (num == u_net_mask) {
			break;
		}
	}
	if (i == 32) {
		return -1;
	} else {
		return 32 - i;
	}
}

int32_t mask_num2str(int32_t mask_num, int8_t *netmask, int32_t netmask_len)
{
	u_int32_t u_net_mask = 0;
	struct in_addr in = {0};
	
	int32_t num = 32 - mask_num;
	if (num < 0) {
		goto err;
	}
	
	u_net_mask = (1 << num) - 1;
	u_net_mask = htonl(u_net_mask);
	in.s_addr = ~u_net_mask;

	if (mask_num == 0) {
		v_strcpy(netmask, netmask_len, "0.0.0.0");
	} else {
		v_strcpy(netmask, netmask_len, inet_ntoa(in));
	}
		
	return 0;
err:
	return -1;
}

/***************************************************************
*
* function: get_ip_by_dev 
* purpose: get specified network card's ip address
* input:
*	ifname 	nic' name,like eth0,eth1,etc
*	ip_str	ptr to ip in string format of ifname
*	ip_int	ptr to ip in int format of ifname
* return:
*	0	ok
*	-1	error
*
****************************************************************/
int32_t get_ip_by_dev(int8_t *ifname, int8_t *ip_str, int32_t len, int32_t *ip_int)	
{
	int32_t s = 0;
	struct ifreq ifr;
	struct sockaddr_in *sin = NULL;

	
	if (NULL == ifname) {
		goto err;
	}	

	bzero(&ifr,sizeof(struct ifreq));
	
	if (-1 == (s = socket(PF_INET, SOCK_STREAM, 0))) {
		goto err;
	}

	strncpy(ifr.ifr_name, ifname, 15);

	if(ioctl(s, SIOCGIFADDR, &ifr) < 0) {
		goto err;
	}

	sin = (struct sockaddr_in *)&ifr.ifr_addr;
	close(s);
	s = 0;

	if (NULL != ip_int) {
		*ip_int = (int32_t)(sin->sin_addr.s_addr);
	}

	if (NULL != ip_str && len > 0) {
		strncpy((char *)ip_str, (char *)inet_ntoa(sin->sin_addr), len);
	}

	return 0;

err:
	if (s != 0) {
		close(s);
		s = 0;
	}
	return -1;
}

/***************************************************************
*
* function: get_mask_by_dev 
* purpose: get specified network card's ip address
* input:
*	ifname 	nic' name,like eth0,eth1,etc
*	ip_str	ptr to ip in string format of ifname
*	ip_int	ptr to ip in int format of ifname
* return:
*	0	ok
*	-1	error
*
****************************************************************/
int32_t get_mask_by_dev(int8_t *ifname, int8_t *mask_str, int32_t len, int32_t *mask_int)	
{
	int32_t s = 0;
	struct ifreq ifr;
	struct sockaddr_in *sin = NULL;

	
	if (NULL == ifname) {
		goto err;
	}	

	bzero(&ifr, sizeof(struct ifreq));
	

	if (-1 == (s = socket(PF_INET, SOCK_STREAM, 0))) {
		goto err;
	}

	strncpy(ifr.ifr_name, ifname, 15);


	if(ioctl(s, SIOCGIFNETMASK, &ifr) < 0) {
		goto err;
	}

	sin = (struct sockaddr_in *)&ifr.ifr_netmask;
	close(s);
	s = 0;

	if (NULL != mask_int) {
		*mask_int = (int32_t)(sin->sin_addr.s_addr);
	}

	if (NULL != mask_str && len > 0) {
		strncpy((char *)mask_str, (char *)inet_ntoa(sin->sin_addr), len);
	}

	return 0;

err:
	if (s != 0) {
		close(s);
		s = 0;
	}
	return -1;
}


int32_t str_split_escape(int8_t *src, int8_t delim,
		struct split_var split_var_table[], int32_t split_var_max)
{
	int32_t len = 0;
	int32_t i = 0;
	int32_t skip = 0;
	int32_t nvar = 0;
	int32_t copyed = 0;

	if (!src || !src[0]) {
		goto err;
	}

	len = strlen(src);
	
	for (i = 0; i < len; i++) {
		if (src[i] == '\\') {
			if (i + 1 < len) {
				if (src[i + 1] == delim) {
					i++;
					copyed += v_strncpy(split_var_table[nvar].addr + copyed, 
							split_var_table[nvar].maxlen,
							src + skip, i - skip - 1);
					if (i == len - 1) {
						v_strncpy(split_var_table[nvar].addr + copyed,
								split_var_table[nvar].maxlen,
								src + i, 1);
						break;
					}
					skip =  i;
				}	
			} else {
				v_strncpy(split_var_table[nvar].addr + copyed,
						split_var_table[nvar].maxlen,
						src + skip, i - skip);
				break;
			}
		} else if (src[i] == delim) {
			v_strncpy(split_var_table[nvar].addr + copyed, 
					split_var_table[nvar].maxlen,
					src + skip, i - skip);
			copyed = 0;
			nvar++;
			if (i == len - 1) {
				break;
			}
			skip = i + 1;
		} else {
			if (i == len - 1) {
				v_strncpy(split_var_table[nvar].addr + copyed,
						split_var_table[nvar].maxlen,
						src + skip, i - skip + 1);
				nvar++;
				break;
			}
		}
		if (nvar == split_var_max) {
			break;
		}
	}

	return nvar;
err:
	return -1;
}
