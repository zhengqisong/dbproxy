/*
 * g_getethinfo.h
 *
 *  Created on: 2013-5-24
 *      Author: qszheng
 */

#ifndef G_GETETHINFO_H_
#define G_GETETHINFO_H_

#define ETHTOOL_GLINK        0x0000000a

#define ETH_PROC_INFO_PATH "/proc/net/dev"

typedef enum { IFSTATUS_UP, IFSTATUS_DOWN, IFSTATUS_ERR } interface_status_t;

struct ethtool_value
{
    int    cmd;
    int    data;
};

typedef struct _eth_info
{
	char name[8];
	int64_t readbit;
	int readpacket;
	int64_t sendbit;
	int sendpacket;
	char ip_mask[32];
	char mac[16];
	int status;
} eth_info;

int get_net_status(char *name);
//int get_eth_ip_and_mask(char *eth_name, char *ip_and_mask);
int get_eth_info(eth_info* interface_info, int max_num, int *num);


#endif /* G_GETETHINFO_H_ */
