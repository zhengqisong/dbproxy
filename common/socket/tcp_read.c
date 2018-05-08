#include "etcp.h"

int tcp_read(int sock, char* buf, int len)
{
	int n = 0;
	n = recv(sock, buf, len, MSG_NOSIGNAL);
	if(0 >= n)
	{
		if(0 == n)
		{
			n = -1;
			goto exit;
		}
		else if(EINTR == errno)
		{
			n = 0;
			goto exit;
		}
		else
		{
			n = -1;
			goto exit;
		}
	}
exit:
	return n;
}
