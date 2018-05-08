#ifndef _G_NUMBER_H
#define _G_NUMBER_H
#include <sys/types.h>

int64_t get_little_endian_bignumber(int8_t *begin, int8_t *end); 
int32_t get_little_endian_number(int8_t *begin, int8_t *end); 
int32_t get_big_endian_number(int8_t *begin, int8_t *end); 

#define MIN(a, b) ((a) > (b) ? (b) : (a))

#endif
