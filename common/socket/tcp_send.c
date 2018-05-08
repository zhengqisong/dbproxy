#include "etcp.h"

int tcp_send(int sock, char* buf, int len)
{
	int total;
	int sent;
	total = 0;
	while (total < len) {
		sent = send(sock, buf + total, len, MSG_NOSIGNAL);
		if(0 > sent){
			if(EINTR == errno){
				continue;
			}else{
				goto err;
			}
		}else{
			total = total + sent;
		}
	}
	return 0;
err:
	return -1;
}
