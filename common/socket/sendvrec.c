#include "etcp.h"

int sendvrec( SOCKET fd, char *bp, size_t len )
{
	int ret = -1;
	u_int32_t size = len + 4;//sizeof(u_int32_t);
	char *data = NULL;
	char *pmsg = NULL;

	data = (char *) malloc(size * sizeof(char));
	if(!data){
		return ret;
	}
	pmsg = data;
	out_uint32_be(pmsg, len);
	out_uint8a(pmsg, bp, len);
	int32_t rc = send(fd, data, size, MSG_NOSIGNAL);
	if(rc == size){
		ret = len;
	}
	free(data);
	pmsg = NULL;
	data = NULL;
	return ret;
}

