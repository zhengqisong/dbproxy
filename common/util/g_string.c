#include <stdio.h>
#include <string.h>

#include "misc.h" /* for macro MIN */
#include "g_optimize.h" /* for macro likely */
#include "g_number.h"

static inline int _v_strncpy(char *dst, int size, const char *src, int len)
{
	int l;
	
	l = MIN(size - 1, len);
	memcpy(dst, src, l);
	dst[l] = 0;

	return l;
}


int v_strncpy(char *dst, int size, const char *src, int len)
{
	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	return _v_strncpy(dst, size, src, len);
}

int v_real_strncpy(char *dst, int size, const char *src, int len)
{
	int l = 0;
	int i = 0;

	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}
	
	l = MIN(size - 1, len);

	for (i = 0; i < l; i++) {
		if (0 == src[i]) {
			break;
		}
		dst[i] = src[i];
	}

	dst[i] = 0;

	return i;
}


int v_wstrncpy(char *dst, int size, const char *src, int len)
{
	int l = 0;
	int i = 0;

	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}
	
	l = MIN(size - 2, len);

	for (i = 0; i + 1 < l; i += 2) {
		if (0 == src[i] && 0 == src[i + 1]) {
			break;
		}
		dst[i] = src[i];
		dst[i + 1] = src[i + 1];
	}

	dst[i] = 0;
	dst[i + 1] = 0;

	return i;
}

int v_strcpy(char *dst, int size, const char *src)
{
	int len = 0;
	
	
	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	len = strlen(src);
	
	if (unlikely(size < 1 || len < 0)) {
		return 0;
	}
	

	return _v_strncpy(dst, size, src, len);
}

static int table[256];//one byte data can represent 256 char

void ShiftTable(const char *pattern, int p_len){
	int i = 0;
	for(i = 0; i < 256; i++) { 
		table[i] = p_len;
	}
	for(i = 0; i < p_len - 1; i++) {
		table[ (u_int8_t)pattern[i] ] = p_len - 1 - i;
	}

}

char *v_memstr(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	if (1 == needle_len) {
		return memchr(haystack, needle[0], range);
	}

	ShiftTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && (needle[m - 1 - k] == haystack[i - k])) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + table[(u_int8_t)haystack[i]]; 
		}
	}

	return NULL;
}

char *v_memrstr(const char *haystack, int range, const char *needle, int needle_len)
{
	char *p = (char *) haystack + range;
	
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	p -= needle_len;

	while(p >= haystack) {
		if(!memcmp(p, needle, needle_len))
			return p;
		p--;
	}

	return NULL;
}

static int nocasetable[256];//one byte data can represent 256 char

void ShiftNocaseTable(const char *pattern, int p_len){
	int i = 0;
	int m = p_len;
	for(i = 0; i < 256; i++) { 
		nocasetable[i] = p_len;
	}
	for(i = 0; i < p_len - 1; i++) {
		nocasetable[ (u_int8_t)pattern[i] ] = p_len - 1 - i;
		if ('a' < (u_int8_t)pattern[i] && (u_int8_t)pattern[i] < 'z') {
			nocasetable[ (u_int8_t)pattern[i] - 32 ] = m - 1 - i;
		} else if ('A' < (u_int8_t)pattern[i] && (u_int8_t)pattern[i] < 'Z') {
			nocasetable[ (u_int8_t)pattern[i] + 32 ] = m - 1 - i;
		}
	}

}

int nocaseequal(char a, char b) 
{
	int ret = 0;

	if (a == b) {
		ret = 1;
	} else if ('a' < a && a < 'z') {
		if (a - 32 == b) {
			ret = 1;
		}
	} else if ('A' < a && a < 'Z') {
		if (a + 32 == b) {
			ret = 1;
		}
	}

	return ret;
}

char *v_nocasememstr(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	if (1 == needle_len) {
		return memchr(haystack, needle[0], range);
	}

	ShiftNocaseTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && nocaseequal(needle[m - 1 - k], haystack[i - k])) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + nocasetable[(u_int8_t)haystack[i]]; 
		}
	}

	return NULL;
}

static int casetable[256];//one byte data can represent 256 char
void ShiftcaseTable(const char *pattern, int p_len){
	int i = 0;
	int j = 0;
	int m = p_len;
	for(i = 0; i < 256; i++) { 
		casetable[i] = m;
	}
	for(j = 0; j < m - 1; j++) {
		casetable[ (u_int8_t)pattern[j] ] = m - 1 - j;
		if ('a' < (u_int8_t)pattern[j] && (u_int8_t)pattern[j] < 'z') {
			casetable[ (u_int8_t)pattern[j] - 32 ] = m - 1 - j;
		}
	}

}

char *v_memcasestr_lowerneedle(const char *haystack, int range, const char *needle, int needle_len)
{
	if(unlikely(needle_len < 1 || !needle))
		return (char *)haystack;

	if(unlikely(needle_len > range || !range || !haystack))
		return NULL;

	ShiftcaseTable(needle, needle_len);
	int m = needle_len;
	int n = range;

	int i = m - 1;
	int k = 0;
	while(i <= n - 1){
		k = 0;                   

		while(k <= m - 1 && (needle[m - 1 - k] == haystack[i - k] || needle[m - 1 - k] == (haystack[i - k] + 32))  ) {
			k++;
		}
		if(k == m) {
			return (char *)(haystack + i - m + 1);
		} else { 
			i = i + casetable[(u_int8_t)haystack[i]]; 
		}
	}

	return NULL;
}

int v_memcpy(void *dst, int size, const void *src, int n)
{
	int l;

	if (unlikely(NULL == dst || NULL == src)) {
		return 0;
	}

	if (unlikely(size < 1 || n < 1)) {
		return 0;
	}

	l = MIN(size, n);
	memcpy(dst, src, l);

	return l;
}

int v_memcat(char *dst, int size, int max_size, const char *src, int len)
{
	int32_t min = 0;

	if (unlikely(!dst || !src || size < 1 || len < 1 || max_size < 1)) {
		goto err;
	}

	min = (max_size - size) < len ? (max_size - size) : len;

	memcpy(dst + size, src, min);
	return min + size;

err:
	return -1;
}

int32_t v_strncat(int8_t *dst, int32_t max_dst_size, int8_t *src)
{
	int32_t src_size = strlen(src);
	int32_t dst_size = strlen(dst);
	int32_t cp_len = 0;

	if (dst_size >= max_dst_size - 1) {
		goto err;
	}

	cp_len = src_size > max_dst_size - dst_size - 1 ? max_dst_size - dst_size - 1 : src_size;
	memcpy(dst + dst_size, src, cp_len);
	dst[cp_len + dst_size] = '\0';

err:
	return cp_len + dst_size;

}


int32_t v_strncat_ex(int8_t *dst, int32_t max_dst_size, int8_t *src, int32_t len)
{
	int32_t src_size = strlen(src);
	int32_t dst_size = strlen(dst);
	int32_t cp_len = 0;

	if (dst_size >= max_dst_size - 1) {
		goto err;
	}

	src_size = src_size > len ? len : src_size;

	cp_len = src_size > max_dst_size - dst_size - 1 ? max_dst_size - dst_size - 1 : src_size;
	memcpy(dst + dst_size, src, cp_len);
	dst[cp_len + dst_size] = '\0';

err:
	return cp_len + dst_size;

}

int32_t v_strncat_comma(int8_t *dst, int32_t max_dst_size, int8_t *src, int32_t src_size)
{
	int32_t dst_size = 0;
	int32_t cp_len = 0;
	
	if (0 == src_size) {
		goto err;
	}

	dst_size = strlen(dst);
	if (0 != dst_size) {
		if (dst_size >= max_dst_size - 2) {
			goto err;
		}
		dst[dst_size] = ',';
		dst_size++;
	} else {
		if (dst_size >= max_dst_size - 1) {
			goto err;
		}
	}

	cp_len = src_size > max_dst_size - dst_size - 1 ? max_dst_size - dst_size - 1 : src_size;
	memcpy(dst + dst_size, src, cp_len);
	dst[cp_len + dst_size] = '\0';
err:
	return cp_len + dst_size;
}

int32_t v_trim_str(int8_t *str)
{
	int32_t  i = 0;
	int32_t  j = 0;
	int32_t len = 0;

	len = strlen(str);
	if (len <= 0) {
		return 0;
	}

	for (i = 0; i < len; i++) {
		if ((*(str+i) == 9 || *(str+i) == 32) == 0) {
			break;
		}
	}
	for (; i < len; i++, j++) {
		*(str+j) = *(str+i);
	}
	*(str+j) = '\0';

	len = strlen(str);
	for (i = len - 1; i >= 0; i--) {
		if ((*(str+i) == 9 || *(str+i) == 32)) {
			*(str+i) = '\0';
		}else{
			break;
		}
	}
	return strlen(str);
}


int32_t v_strspn(int8_t* string, int32_t string_len, int8_t* control)
{
	if (NULL == string || NULL == control || string_len <= 0) {
		return -1;
	}
	u_int8_t *str = (u_int8_t*)string;
	u_int8_t *ctrl = (u_int8_t *)control;
	u_int8_t  map[32] = {0};
	int32_t count = 0;

	/* Clear out bit map */
	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}

	/* Set bits in control map */
	while (*ctrl) {//register each char in control string set  
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++;
	}

	/* 1st char NOT in control map stops search */
	if (*str) {//check each char in string, comparing the each char in the register table 
		count = 0;
		while (count < string_len && (map[*str >> 3] & (1 << (*str & 7)))) {
			count++;
			str++;
		}
		return(count);
	}
	return(0);
}


int32_t v_strcspn(int8_t* string, int32_t string_len, int8_t* control)
{
	if (NULL == string || NULL == control || string_len <= 0) {
		return -1;
	}
	u_int8_t *str = (u_int8_t*)string;
	u_int8_t *ctrl = (u_int8_t*)control;
	u_int8_t  map[32] = {0};
	int32_t count = 0;

	/* Clear out bit map */
	for (count = 0; count < 32; count++) {
		map[count] = 0;
	}

	/* Set bits in control map */
	while (*ctrl) {//register each char in control string set  
		map[*ctrl >> 3] |= (1 << (*ctrl & 7));
		ctrl++;
	}

	map[0] |= 1;    /* null chars not considered */
	/* 1st char NOT in control map stops search */
	if (*str) {//check each char in string, comparing the each char in the register table 
		count = 0;
		while (count < string_len && (!(map[*str >> 3] & (1 << (*str & 7))))) {
			count++;
			str++;
		}
		return(count);
	}

	return(0);
}


int32_t v_is_digit_string(int8_t *str, int32_t str_len)
{
	int32_t i = 0;

	if (NULL == str || 0 >= str_len) {
		goto err;
	}

	for (i = 0; i < str_len; i++) {
		if (str[i] < '0' || str[i] > '9') {
			goto err;
		}
	}
	
	return 0;
err:
	return -1;
}

int32_t v_strcasecmp(int8_t *text, int32_t text_len, int8_t *pattern, int32_t pattern_len) 
{
	int32_t	ret = 0;
	int32_t	i = 0;

	if (NULL == text || NULL == pattern || pattern_len <= 0 || pattern_len != text_len) {
		ret = 1;
		goto err;
	}

	for (i = 0; i < pattern_len; i++) {
		if (text[i] == pattern[i]) {
			continue;
		} else if (('a' < text[i] && text[i] < 'z') || ('A' < text[i] && text[i] < 'Z')) {
			if (('a' < pattern[i] && pattern[i] < 'z') || ('A' < pattern[i] && pattern[i] < 'Z')) {
				if (((text[i] - 32) == pattern[i]) || ((text[i] + 32) == pattern[i])) {
					continue;
				} else {
					ret = 1;
					break;
				}
			} else {
				ret = 1;
				break;
			}
		} else {
			ret = 1;
			break;
		}
	}

err:
	return ret;
}

int32_t v_strncmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len)
{
	int32_t len = 0;

	if (NULL == dst || NULL == src || dst_len < 0 || src_len < 0) {
		return -1;
	}

	len = MIN(dst_len, src_len);

	return strncmp(dst, src, len);
}

int32_t v_strncasecmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len)
{
	int32_t len = 0;

	if (NULL == dst || NULL == src || dst_len < 0 || src_len < 0) {
		return -1;
	}

	len = MIN(dst_len, src_len);

	return strncasecmp(dst, src, len);
}

int32_t v_atoi(int8_t *begin, int32_t data_len)
{
	int32_t size = 0;
	int8_t *data = begin;
	int8_t *end = data + data_len;
	int32_t sign = 1;

	if (data < end && '-' == *data) {
		sign = -1;
		data++;
	}

	for (; data < end; data++) {
		if (*data >= '0' && *data <= '9') {
			size = size * 10 + *data - '0';
		} else {
			break;
		}
	}

	return size * sign;

}

int32_t v_safe_atoi(int8_t *begin, int32_t data_len, int32_t *value)
{
	int32_t size = 0;
	int8_t *data = begin;
	int8_t *end = data + data_len;
	int32_t sign = 1;

	if (data < end && '-' == *data) {
		sign = -1;
		data++;
	}

	for (; data < end; data++) {
		if (*data >= '0' && *data <= '9') {
			size = size * 10 + *data - '0';
		} else {
			goto err;
		}
	}

	*value = size * sign;
	return 0;
err:
	return -1;

}

//clear first byte of each row of a string array
int32_t	v_strarray_mem_clear(int8_t *strarray, int32_t row_num, int32_t column_num)
{
	int32_t i = 0;
	if (NULL == strarray || 0 >= row_num || 0 >= column_num) {
		goto err;
	}

	for (i = 0; i < row_num; i++) {
		strarray[i * column_num] = '\0';
	}

	return 0;
err:
	return -1;
}

int32_t v_strlen(int8_t *begin, int32_t data_len)
{
	if (NULL == begin || data_len <= 0) {
		return 0;
	}

	return MIN(strlen(begin), data_len);
}

int8_t *v_memchr_escape(int8_t *s, int32_t c, int32_t n)
{
	int32_t i = 0;
	for (i = 0; i < n; i++) {
		if ((int32_t)s[i] == '\\' && i < n - 1 && (int32_t)s[i + 1] == c) {
			i++;
		} else if (s[i] == c) {
			break;
		} 
	}	
	if (i == n) {
		return NULL;
	} else {
		return s + i;
	}
}


int32_t v_str_replace(int8_t *src, int32_t src_len, int8_t *find_str, int8_t *replace_str, int8_t *dest, int32_t dest_len)
{
	int8_t *begin = src;
	int8_t *end = src + src_len;
	int8_t *itor1 = begin;
	int8_t *itor2 = begin;
	int32_t find_len = strlen(find_str);
	int32_t ret = 0;

	if(NULL == src || NULL == dest || 0 == dest_len || NULL == find_str || NULL == replace_str){
		goto out;
	}

	while(itor1 < end) {
		// find next
		itor2 = strstr(itor1, find_str);
		if (NULL == itor2) {
			break;
		}
		// replace one
		v_strncat_ex(dest, dest_len, itor1, (itor2 - itor1));
		ret = v_strncat(dest, dest_len, replace_str);
		itor1 = itor2 + find_len;
	}

	if (itor1 < end) {
		ret = v_strncat(dest, dest_len, itor1);
	}

out:
	return ret;
}


/**
*  change string to HexString
*  
*  such as : "abcABC" -->  "61 62 63 41 42 43"
*
**/
void v_str_2_hexstr(u_int8_t *src_str, int32_t src_length, u_int8_t *dst_str, int32_t max_length)
{
		
	u_int32_t offset = 0;
	u_int32_t i = 0;

	//int32_t src_length = strlen(src_str);
	for(i=0; i<src_length && offset<(max_length-3); i++){
		if(*(src_str+i) == 0){
			offset += sprintf(dst_str+offset, "%s ", "00" );

		} else if( *(src_str+i)>0  && *(src_str+i)<='F' ){
			offset += sprintf(dst_str+offset, "0%X ", *(src_str+i) );

		} else {
			offset += sprintf(dst_str+offset, "%2X ", *(src_str+i) );
		}		
	}
	*(dst_str+offset)=0;
}


/**
*  find char from string
*  return NULL or u_int8_t*
*
**/
u_int8_t* v_strchr(u_int8_t *src_str, int32_t length, u_int8_t c)
{
	int32_t i = 0;	
	for(i=0; i<length; i++){
		if(src_str[i] == c){
			return &src_str[i];
		};
	}
	return NULL;
}

/** 
 * v_move_front: move array element front 
 *
 *  *** input ***
 * data: 0 1 2 3 4 5 6 7 8
 *  length: 9
 *  begin: 3 (zero from)
 *  end: 5 (zero from)
 * 
 *  *** output ***
 * data: 0 1 2 6 7 8
 * length: 6
 * 
**/
int32_t v_move_front(u_int8_t *data, int32_t *length, int32_t begin, int32_t end)
{
	int32_t result = 0;
	int32_t offset = 0;
	
	if(begin >= end){
		goto ret;
	}

	offset = end - begin;
	*length -= offset;

	int32_t i = 0;
	for(i=0; i<*length; i++){		
		data[begin+i] = data[begin+i+offset];
	}	
	result = 1;
			
ret:
	return result;
}


int32_t g_memstr(u_int8_t *src, int32_t src_len, u_int8_t *substr, int32_t substr_len)
{
 	int i = 0;
 	int j = 0;
 	int mark = 0;

 	while(i < src_len - substr_len + 1)
 	{
 		j = 0;
 		mark = 0;
 		while(j < substr_len)
 		{
 			if(src[i+j] != substr[j])
 			{
 				mark = 1;
 				break;
 			}
 			j++;
 		}

 		if(!mark)
 		{
 			return i;
 		}
 		i++;
 	}
 	return -1;
}

int32_t g_memcasestr(u_int8_t *src, int32_t src_len, u_int8_t *substr, int32_t substr_len)
{
 	int i = 0;
 	int j = 0;
 	int mark = 0;
 	u_int8_t ch1,ch2;

 	while(i < src_len - substr_len + 1)
 	{
 		j = 0;
 		mark = 0;
 		while(j < substr_len)
 		{
 			ch1 = src[i+j];
 			ch2 = substr[j];
 			if(ch1 >= 'A' && ch1 <= 'Z')
 			{
 				ch1 += 32;
 			}
 			if(ch2 >= 'A' && ch2 <= 'Z')
			{
 				ch2 += 32;
			}

 			if(ch1 != ch2)
 			{
 				mark = 1;
 				break;
 			}
 			j++;
 		}

 		if(!mark)
 		{
 			return i;
 		}
 		i++;
 	}
 	return -1;
}

int byte2hexchar(char *dst, char *src, int len)
{
	int i = 0;

	char tmp[4]={0};
	char *p;
	p = dst;
	for(i = 0; i < len; i++)
	{
		sprintf(tmp, "%02x", (unsigned char)(*(src + i)));
		memcpy(p, tmp, 2);
		p += 2;
	}
	return len * 2;
}

int hexchar2byte(char *dst, char *src, int len)
{
	int i = 0;
	int num = 0;
	char tmp[4]={0};
	char *p;
	p = dst;
	if(len % 2)
	{
		return 0;
	}
	for(i = 0; i < len; i+=2)
	{
		memcpy(tmp, &src[i], 2);
		sscanf(tmp, "%02x", &num);
		*p = (char)num;
		p++;
	}
	return len/2;
}
