/*
 * g_getethinfo.c
 *
 *  Created on: 2013-5-24
 *      Author: qszheng
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <errno.h>
#include <unistd.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include "g_getethinfo.h"


static int get_mask_number(unsigned int addr_number)
{
	int i = 0;
	unsigned int mask_index = 0;
	unsigned int cur_number = 1;
	for (i = 0; i < 32; i++)
	{
		if (0 != (addr_number & (cur_number << i)))
		{
			break;
		}
	}
	mask_index = 32 - i;
	return mask_index;
}

int get_net_status(char *name)
{
	int ret = 0;
	int fd = -1;
	struct ifreq ifr;
	struct ethtool_value edata;

	if(NULL == name)
	{
		ret = -1;
		goto err;
	}
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
	{
		ret = -1;
		goto err;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, name, sizeof(ifr.ifr_name)-1);

	edata.cmd = ETHTOOL_GLINK;
	ifr.ifr_data = (caddr_t) &edata;

	ret = ioctl(fd, SIOCETHTOOL, &ifr);
	if (0 == ret)
	{
		ret = edata.data ? 1 : 0;
	}

err:
	if (-1 != fd)
	{
        close(fd);
		fd = 0;
	}
    return ret;
}

static int get_eth_ip_and_mask(char *eth_name, char *ip_and_mask, char* mac)
{
	struct sockaddr_in *addr;
	struct ifreq ifr;
	char *address = NULL;
	char ip_addr[32] = {0};
	int sockfd = -1;
	int mask_index = 0;
	int ret = 0;

	if (NULL == eth_name || NULL == ip_and_mask)
	{
		ret = -1;
		goto err;
	}
	sockfd = socket(AF_INET,SOCK_DGRAM, 0);
	if (-1 == sockfd)
	{
		ret = -1;
		goto err;
	}
	strncpy(ifr.ifr_name, eth_name, sizeof(ifr.ifr_name)-1);

	if (ioctl(sockfd, SIOCGIFADDR, &ifr) == -1)
	{
		ret = -1;
		goto err;
	}
	addr = (struct sockaddr_in *)&(ifr.ifr_addr);
	address = inet_ntoa(addr->sin_addr);
	strncpy(ip_addr, address, strlen(address));

	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr) == -1)
	{
		ret = -1;
		goto err;
	}
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	mask_index = get_mask_number(ntohl(addr->sin_addr.s_addr));
	snprintf(ip_and_mask, 32, "%s/%d", ip_addr, mask_index);

	snprintf(mac, 16, "%02x-%02x-%02x-%02x-%02x-%02x",
	(unsigned char)ifr.ifr_hwaddr.sa_data[0],
	(unsigned char)ifr.ifr_hwaddr.sa_data[1],
	(unsigned char)ifr.ifr_hwaddr.sa_data[2],
	(unsigned char)ifr.ifr_hwaddr.sa_data[3],
	(unsigned char)ifr.ifr_hwaddr.sa_data[4],
	(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
err:
	if (sockfd >= 0)
	{
		close(sockfd);
		sockfd = 0;
	}
	return ret;
}

static int fill_eth_info(char *buf, int max_buf_len, eth_info *interface_info)
{
	//only get the 1, 2, 9, 10 info
	char *ptr = buf;
	char *ptr_head = NULL;
	char *ptr_tail = NULL;
	char *buf_end = NULL;
	char index = 0;
	char eth_value[16] = {0};

	if ((NULL == buf) || (NULL == interface_info)) {
		goto err;
	}

	buf_end = buf + max_buf_len;

	while(10 > index) {
		while (*ptr == ' ' && ptr < buf_end) {
			ptr++;
		}

		if (ptr == buf_end) {
			goto err;
		}

		ptr_head = ptr;

		while (*ptr != ' ' && ptr < buf_end) {
			ptr++;
		}

		if (ptr == buf_end) {
			goto err;
		}

		ptr_tail = ptr;

		index++;

		strncpy(eth_value, ptr_head, (ptr_tail - ptr_head));

		switch (index) {
			case 1:
				interface_info->readbit = 8 * atoll(eth_value);
				break;
			case 2:
				interface_info->readpacket = atoi(eth_value);
				break;
			case 9:
				interface_info->sendbit = 8 * atoll(eth_value);
				break;
			case 10:
				interface_info->sendpacket = atoi(eth_value);
				break;
			default:
				break;
		}
		bzero(eth_value, sizeof(eth_value));
	}
	return 0;

err:
	return -1;
}

int get_eth_info(eth_info* interface_info, int max_num, int *num)
{
	FILE *fd = NULL;
	char buf[1024] ={0};
	char eth_name[8] ={0};
	char tmp_eth_name[8] ={0};
	int ret = 0;
	char *ptr = NULL;
	char *ptr_head = NULL;
	char *ptr_tail = NULL;
	char *buf_end = NULL;
	int net_status = 0;

	fd = fopen(ETH_PROC_INFO_PATH, "r");
	if (NULL == fd) {
		ret = -1;
		goto err;
	}

	/* jump two lines  */
	if (NULL == fgets(buf, sizeof(buf), fd)) {
		ret = -1;
		goto err;
	}

	if (NULL == fgets(buf, sizeof(buf), fd)) {
		ret = -1;
		goto err;
	}
	*num = 0;

	while (0 == feof(fd)) {
		fgets(buf, sizeof(buf), fd);
		ptr = buf;
		buf_end = buf + strlen(buf);
		/* in each line , get the head info */
		while (' ' == *ptr && ptr < buf_end) {
			ptr++;
		}

		if (ptr == buf_end) {
			ret = -1;
			goto err;
		}

		ptr_head = ptr;

		while (*ptr != ':' && ptr < buf_end) {
			ptr++;
		}

		if (ptr == buf_end) {
			ret = -1;
			goto err;
		}

		ptr_tail = ptr;

		strncpy(eth_name, ptr_head, (ptr_tail - ptr_head));

		if (0 == strncmp(eth_name, "lo", 2)) {
			continue;
		}

		//not fill the same eth, just in case
		if (0 == strncmp(eth_name, tmp_eth_name, 8)) {
			continue;
		}

		strncpy(interface_info[*num].name, eth_name, strlen(eth_name));

		/* keep a copy for next compared */
		strncpy(tmp_eth_name, interface_info[*num].name, 8);

		/* get the interface infomation */
		if (-1 == fill_eth_info(ptr_tail + 1, buf_end - (ptr_tail + 1),
					&(interface_info[*num])))
		{
			ret = -1;
			goto err;
		}
		//get net status
		if (-1 == (net_status = get_net_status(eth_name)))
		{
			ret = -1;
			goto err;
		}

		interface_info[*num].status = net_status;
		if (-1 == get_eth_ip_and_mask(eth_name, interface_info[*num].ip_mask, interface_info[*num].mac)) {
		}

		(*num)++;
		if (*num >= max_num) {
			ret = -1;
			goto err;
		}
	}

err:
	if (NULL != fd) {
		fclose(fd);
		fd = NULL;
	}

	return ret;
}

