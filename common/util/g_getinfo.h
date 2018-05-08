/*
 * g_getinfo.h
 *
 *  Created on: 2013-5-21
 *      Author: qszheng
 */

#ifndef G_GETINFO_H_
#define G_GETINFO_H_


#define _PATH_PROC_MEMINFO "/proc/meminfo"
#define _PATH_PROC_STAT "/proc/stat"
#define _PATH_CPUINFO "/var/tmp/cpuinfo"
#define _PATH_DISKUSEINFO "/var/tmp/diskuseinfo"
#define _EXEC_CPUINFO "cat /proc/cpuinfo |grep 'model name' |awk -F : '{print $2}' > /var/tmp/cpuinfo"
#define _EXEC_DISK "df -h > /var/tmp/diskuseinfo"

typedef struct _CPU_STAT         //定义一个cpu occupy的结构体
{
	char name[20];      //定义一个char类型的数组名name有20个元素
	unsigned long user; //定义一个无符号的int类型的user
	unsigned long nice; //定义一个无符号的int类型的nice
	unsigned long system;//定义一个无符号的int类型的system
	unsigned long idle; //定义一个无符号的int类型的idle
	unsigned long iowait; //定义一个无符号的int类型的iowait
	unsigned long irq; //定义一个无符号的int类型的irq
	unsigned long softirq; //定义一个无符号的int类型的softirq
}CPU_STAT;

int get_mem_used(float *used,int *totalmem);
int get_cpu_stat(CPU_STAT *cpustat, char *name);
float calc_cpu_used(CPU_STAT *o, CPU_STAT *n);

int get_cpuinfo(char *buf, int len);
int get_diskuseinfo(char *buf, int len);


#endif /* G_GETINFO_H_ */
