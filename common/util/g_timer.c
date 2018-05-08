#include "g_timer.h"
#include <time.h>

int64_t get_milli_time()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return ((int64_t)tv.tv_sec) * 1000 + ((int64_t)tv.tv_usec) / 1000;
}

int64_t get_u_time()
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	return ((int64_t)tv.tv_sec) * 1000 * 1000 + ((int64_t)tv.tv_usec);
}

int32_t get_time()
{
	return time(NULL);
}
