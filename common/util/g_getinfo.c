/*
 * g_getinfo.c
 *
 *  Created on: 2013-5-21
 *      Author: qszheng
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include<stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include "g_getinfo.h"
#include "trace.h"

int readfile(char *filename, char *buf, int len)
{
	int fd, num_read;
	if ( (fd  = open(filename, O_RDONLY, 0)) == -1 ){
		return -1;
	}
	if ( (num_read = read(fd, buf, len - 1)) <= 0 ) {
		num_read = -1;
	} else {
		buf[num_read] = 0;
	}
	return num_read;
}

int get_mem_used(float* used, int* totalmem)
{
	FILE *fd = NULL;
	char name[30];
	char unit[16];
	char buf[512];
	int digital = 0;
	int total=-1;
	int free=-1;
	int buff=-1;
	//float tmp_used;


	fd = fopen(_PATH_PROC_MEMINFO, "r");
	if (!fd)
		return -1;


	while (fgets(buf, sizeof buf, fd))
	{
		sscanf(buf, "%s%d%s", name, &digital, unit);
		if(!strcmp(name,"MemTotal:")){
			total = digital;
			*totalmem = total;
		}
		if(!strcmp(name, "MemFree:")){
			free  = digital;
		}
		if(!strcmp(name, "Buffers:")){
			buff = digital;
		}
	}
	fclose(fd);

	if ( total != -1 && free != -1)
	{
		*used=(float)((float)total-free-buff)/total;
	}
	return 0;
}

int get_cpu_stat(CPU_STAT *cpustat, char *name)
{
	FILE *fd = NULL;
	char buf[512];

	fd = fopen(_PATH_PROC_STAT, "r");
	if (!fd)
		return -1;

	while (fgets(buf, sizeof buf, fd))
	{
		sscanf(buf, "%s %lu %lu %lu %lu %lu %lu %lu", cpustat->name, &cpustat->user,
				&cpustat->nice, &cpustat->system, &cpustat->idle, &cpustat->iowait,
				&cpustat->irq, &cpustat->softirq);
		//dbgmsg("name=%s",name);
		if(cpustat->name && strcmp(name, cpustat->name) == 0)
		{
			return 0;
		}
	}
	return -2;
}

float calc_cpu_used(CPU_STAT *o, CPU_STAT *n)
{
	float cpu_now_used = 0.0;
	int dlta_user_cpu = 0;
	int dlta_nice_cpu = 0;
	int dlta_sys_cpu = 0;
	int dlta_idle_cpu = 0;
	int dlta_iowait_cpu = 0;
	int dlta_irq_cpu = 0;
	int dlta_softirq_cpu = 0;

	int sum = 0;

	dlta_user_cpu = n->user - o->user;
	dlta_nice_cpu = n->nice - o->nice;
	dlta_sys_cpu = n->system - o->system;
	dlta_idle_cpu = n->idle - o->idle;
	dlta_iowait_cpu = n->iowait - o->iowait;
	dlta_irq_cpu = n->irq - o->irq;
	dlta_softirq_cpu = n->softirq - o->softirq;

	sum = dlta_user_cpu + dlta_nice_cpu + dlta_sys_cpu +
			dlta_idle_cpu + dlta_iowait_cpu +
			dlta_irq_cpu + dlta_softirq_cpu;
	if (0 != sum)
	{
		cpu_now_used = ((float)(sum - dlta_idle_cpu))/(float)(sum);
	}
	return cpu_now_used;
}


int get_cpuinfo(char *buf, int len)
{
	int rt = 0;

	rt = system(_EXEC_CPUINFO);
	if(rt <= 0)
	{
		buf[0] = 0;
		return 0;
	}

	return readfile(_PATH_CPUINFO, buf, len);
}

int get_diskuseinfo(char *buf, int len)
{
	int rt = 0;

	rt = system(_EXEC_DISK);
	if(rt <= 0)
	{
		buf[0] = 0;
		return 0;
	}

	return readfile(_PATH_DISKUSEINFO, buf, len);
}

