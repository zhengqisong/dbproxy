#include <stdio.h>
#include <sys/types.h>
#include <assert.h>

int64_t get_little_endian_bignumber(int8_t *begin, int8_t *end) 
{
	switch (end - begin) {
		case 1:
			return (int64_t)(u_int8_t)begin[0];
		case 2:
			return (int64_t)(u_int8_t)begin[0] + (((int64_t)(u_int8_t)begin[1]) << 8);
		case 3:
			return (int64_t)(u_int8_t)begin[0] + (((int64_t)(u_int8_t)begin[1]) << 8)
				+ (((int64_t)(u_int8_t)begin[2]) << 16);
		case 4:
			return (int64_t)(u_int8_t)begin[0] + (((int64_t)(u_int8_t)begin[1]) << 8)
				+ (((int64_t)(u_int8_t)begin[2]) << 16)
				+ (((int64_t)(u_int8_t)begin[3]) << 24);
		case 8:
			return (int64_t)(u_int8_t)begin[0] + (((int64_t)(u_int8_t)begin[1]) << 8)
				+ (((int64_t)(u_int8_t)begin[2]) << 16)
				+ (((int64_t)(u_int8_t)begin[3]) << 24)
				+ (((int64_t)(u_int8_t)begin[4]) << 32)
				+ (((int64_t)(u_int8_t)begin[5]) << 40)
				+ (((int64_t)(u_int8_t)begin[6]) << 48)
				+ (((int64_t)(u_int8_t)begin[7]) << 56);
		default:
			return 0;
	}
}

int32_t get_little_endian_number(int8_t *begin, int8_t *end) 
{
	assert(end - begin == 1 || end - begin == 2 || end - begin == 3 || end - begin == 4);

	switch (end - begin) {
		case 1:
			return (int32_t)(u_int8_t)begin[0];
		case 2:
			return (int32_t)(u_int8_t)begin[0] + (((int32_t)(u_int8_t)begin[1]) << 8);
		case 3:
			return (int32_t)(u_int8_t)begin[0] + (((int32_t)(u_int8_t)begin[1]) << 8)
				+ (((int32_t)(u_int8_t)begin[2]) << 16);
		case 4:
			return (int32_t)(u_int8_t)begin[0] + (((int32_t)(u_int8_t)begin[1]) << 8)
				+ (((int32_t)(u_int8_t)begin[2]) << 16)
				+ (((int32_t)(u_int8_t)begin[3]) << 24);
		default:
			return 0;
	}
}


int32_t get_big_endian_number(int8_t *begin, int8_t *end) 
{
	switch (end - begin) {
		case 1:
			return (int32_t)(u_int8_t)begin[0];
		case 2:
			return (int32_t)(u_int8_t)begin[1] 
				+ (((int32_t)(u_int8_t)begin[0]) << 8);
		case 3:
			return (int32_t)(u_int8_t)begin[2]
				+ (((int32_t)(u_int8_t)begin[1]) << 8)
				+ (((int32_t)(u_int8_t)begin[0]) << 16);
		case 4:
			return (int32_t)(u_int8_t)begin[3] 
				+ (((int32_t)(u_int8_t)begin[2]) << 8)
				+ (((int32_t)(u_int8_t)begin[1]) << 16)
				+ (((int32_t)(u_int8_t)begin[0]) << 24);
		default:
			return 0;
	}
}
