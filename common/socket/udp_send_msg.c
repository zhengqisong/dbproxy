#include "etcp.h"
/*
 * udp_send_msg.c
 *
 *  Created on: 2013-8-7
 *      Author: qszheng
 */

int send_udp_msg(void * msg, int msg_size, char * szaddr, int port)
{	int udp_sock = 0;
	struct sockaddr_in addrin;
	if((udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		fprintf(stderr, "socket process failed!\n");
		return -1;
	}
	memset(&addrin, 0, sizeof(struct sockaddr_in));
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = inet_addr(szaddr);
	addrin.sin_port = htons(port);

	sendto(udp_sock, msg, msg_size, 0, (struct sockaddr *)&addrin, sizeof(addrin));
	close(udp_sock);
	return 0;
}

