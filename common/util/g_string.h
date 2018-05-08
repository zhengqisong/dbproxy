#ifndef _G_STRING_H
#define _G_STRING_H
#include <sys/types.h>

int v_strncpy(char *dst, int size, const char *src, int len);
int v_strcpy(char *dst, int size, const char *src);
int v_real_strncpy(char *dst, int size, const char *src, int len);
int v_wstrncpy(char *dst, int size, const char *src, int len);
/* 
 *	Find the first occurrence of the substring.
 */
char *v_memstr(const char *haystack, int range, const char *needle, int needle_len);
char *v_nocasememstr(const char *haystack, int range, const char *needle, int needle_len);
/* 
 *	Find the first occurrence of the substring, no case sense
 */
char *v_memcasestr_lowerneedle(const char *haystack, int range, const char *needle, int needle_len);

/* 
 *	Find the last occurrence of the substring.
 */
char *v_memrstr(const char *haystack, int range, const char *needle, int needle_len);

int v_memcpy(void *dst, int size, const void *src, int n);

int v_memcat(char *dst, int size, int max_size, const char *src, int len);
int32_t v_strncat(int8_t *dst, int32_t max_size, int8_t *src);
int32_t v_strncat_ex(int8_t *dst, int32_t max_size, int8_t *src, int32_t len);

//int32_t venus_trim_str(int8_t *str);

int32_t v_strspn(int8_t* string, int32_t string_len, int8_t* control);
int32_t v_strcspn(int8_t* string, int32_t string_len, int8_t* control);

int32_t v_is_digit_string(int8_t *str, int32_t str_len);
int32_t v_strcasecmp(int8_t *text, int32_t text_len, int8_t *pattern, int32_t pattern_len); 
int32_t v_atoi(int8_t *data, int32_t data_len);
int32_t v_safe_atoi(int8_t *begin, int32_t data_len, int32_t *value);
int32_t	v_strarray_mem_clear(int8_t *strarray, int32_t row_num, int32_t column_num);
int32_t v_strlen(int8_t *begin, int32_t data_len);
int32_t v_strncat_comma(int8_t *dst, int32_t max_dst_size, int8_t *src, int32_t src_size);
int32_t v_strncmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len);
int32_t v_strncasecmp(int8_t *dst, int32_t dst_len, int8_t *src, int32_t src_len);

int8_t *v_memchr_escape(int8_t *s, int32_t c, int32_t n);
int32_t v_str_replace(int8_t *src, int32_t src_len, int8_t *find_str, int8_t *replace_str, int8_t *dest, int32_t dest_len);

/**
*  change string to HexString
*  
*  such as : "abcABC" -->  "61 62 63 41 42 43"
*
**/
void v_str_2_hexstr(u_int8_t *src_str, int32_t src_length, u_int8_t *dst_str, int32_t max_length);

/**
*  find char from string
*  return NULL or u_int8_t*
*
**/
u_int8_t* v_strchr(u_int8_t *src_str, int32_t length, u_int8_t c);

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
int32_t v_move_front(u_int8_t *data, int32_t *length, int32_t begin, int32_t end);

int32_t g_memstr(u_int8_t *src, int32_t src_len, u_int8_t *substr, int32_t substr_len);

int byte2hexchar(char *buf, char *src, int len);

int hexchar2byte(char *buf, char *src, int len);

int32_t v_trim_str(int8_t *str);

#endif
