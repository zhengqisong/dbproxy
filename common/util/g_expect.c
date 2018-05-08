#include "g_expect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/resource.h>

#include <ctype.h>
#include <fcntl.h>
#include <regex.h>

int expect(int in, char *checklist[], int list_len, int timeout, char *buf, int *buf_len)
{
	int ret = -1;
	int n;
	int m;

	fd_set fds;
	regex_t reg;
	int nm = 10;
	regmatch_t pmatch[nm];
	struct timeval tv;
	int retval;

	int times = timeout;

	tv.tv_sec = 1;
	tv.tv_usec = 0;
	fprintf(stderr,"g_expect in\n");
	while(1)
	{
		if(times < 0)
		{
			goto end;
		}
		FD_ZERO(&fds);
		FD_SET(in, &fds);
		memset(buf, '\0', *buf_len);
		fprintf(stderr, "begin select in=%d\n",in);
		//retval = select(FD_SETSIZE, &fds, NULL, NULL, &tv);
		retval = select(in + 1, &fds, NULL, NULL, &tv);
		fprintf(stderr, "timeout = %d\n",times);
		fprintf(stderr, "%d\n",retval);
		if(retval ==-1)
		{
			if(errno == EINTR)
				continue;
			ret = -1;
			goto end;
		}
		else if(retval == 0)
		{
			times--;
			continue;
		}


		//read data
		if((n = read(in, buf, *buf_len - 1)) <= 0)
		{
			if(n == 0)
			{
				ret = -1;
				goto end;
			}
			if(errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			else
			{
				ret = -1;
				goto end;
			}
		}
		for(m = 0; m < list_len; m++)
		{
			if(regcomp(&reg, checklist[m] ,REG_EXTENDED|REG_ICASE|REG_NEWLINE) == 0 && regexec(&reg, buf, nm, pmatch, 0) == 0 )
			{
				ret = m;
				*buf_len = n;
				//regfree(&reg);
				goto end;
			}
		}
		fprintf(stderr,"ret=%d:%s\n", ret, buf);
	}
end:
	regfree(&reg);
	fprintf(stderr,"g_expect exit\n");
	return ret;
}
