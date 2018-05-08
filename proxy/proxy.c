#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <getopt.h>
#include <pthread.h>
#include <poll.h>

#include "trace.h"
#include "MiscDecoder.h"
#include "g_getinfo.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "g_uuid.h"
#include "etcp.h"
#include "misc.h"
#include "g_string.h"
#include "sys_conf.h"
#include "proto_parser_conf.h"
#include "session.h"
#include "mysql_cache.h"
#include "redis_cache.h"

#define __PROCESS__
extern char **environ; 

int rds_proxy_argc;
char **rds_proxy_argv;

static SOCKET listen_socket = -1;
SYS_CONF sys_conf;

typedef struct _argv_conf {
	char ip[32];
	char port[8];
	char config[256];
	char deamon[5];
} argv_conf;

static void printhelp(){
	printf("Usage1: ./Proxy [-Options1] [-Options2]\n");
	printf("             Use config file if no any argument or ,it proxies any IP address and the default Port is 80\n\n");
	printf("            Options1:\n");
	printf("             -i,--ip   	 it is the IP address for Proxy,valid option:\n");
	printf("                                 **.**.**.**  normal IP address such as 192.168.100.1\n");
	printf("                                 ANY              this option means it proxies  any IP address\n\n");
	printf("            Options2:\n");
	printf("            -p,--port       it is the port for proxy,valid option:such as 80\n\n\n");
	
	printf("Usage2: ./Proxy Options1 [Options2]\n");
	printf("            Options1:\n");
	printf("                         it is the IP address for Proxy,valid option:\n");
	printf("                                 **.**.**.**  normal IP address such as 192.168.100.1\n");
	printf("                                 ANY              this option means it proxies  any IP address\n\n");
	printf("            Options2:\n");
	printf("                         it is the port for proxy,valid option:such as 80\n\n\n");
	
	exit(0);
}

static void parse_regargs(int argc, char **argv, argv_conf *run_argv)
{
	struct option longopts[] = {
		{"ip",   required_argument, NULL, 'i'},
		{"port", required_argument, NULL, 'p'},
		{"conf", required_argument, NULL, 'c'},
		{"deamon", required_argument, NULL, 'd'},
		{"help", no_argument,       NULL, 'h'},
		{0, 0, 0, 0}
	};
	int c = 0;
	
	char *ip = NULL; char *port = NULL;
	char *conf = NULL;char *deamon = NULL;
	while(-1 != (c = getopt_long(argc, argv, ":i:p:c:f:d:hW;", longopts, NULL))){
		switch(c){
			case 'i':
				ip = strcasecmp("ANY", optarg)? optarg : NULL;
				if(ip){
    				memcpy(run_argv->ip, ip, strlen(ip) > 30 ? 30 : strlen(ip));
    			}
				break;
			case 'p':
				port = optarg;
				memcpy(run_argv->port, port, strlen(port) > 6 ? 6 : strlen(port));
				break;
		    case 'c':
		        conf = optarg;
		        memcpy(run_argv->config, conf, strlen(conf) > 255 ? 255 : strlen(conf));
		        break;
		    case 'd':
		        deamon = optarg;
		        memcpy(run_argv->deamon, deamon, strlen(deamon) > 4 ? 4 : strlen(deamon));
		        break;
			case 'h':
				printhelp();
				break;
			case 0:
				break;
			case ':':
				fprintf(stderr, "%s: option '-%c' requires an argument\n", argv[optind], optopt);
				break;
			case '?':
			default:
				fprintf(stderr, "%s: option '-%c' is invalid: ignored\n", argv[optind], optopt);
		}
	}
}

static void stop_listen()
{
	CLOSE_SOCKET(listen_socket);
	listen_socket = -1;
}

static void sa_handler_SIGHUP(int signo)
{
	int old_errno = errno;
	stop_listen();
	errno = old_errno;
}

int proxy_init(int argc, char **argv, argv_conf *run_argv)
{
    struct rlimit rl_file;
	struct rlimit rl_proc;
	char *ip = NULL;
	char *port = NULL;
	
	if ((3 == argc) && ('-' != argv[1][0]))
	{
		ip = strcasecmp("ANY", argv[1])? argv[1] : NULL;
		port = argv[2];
		if(ip){
    		memcpy(run_argv->ip, ip, strlen(ip) > 30 ? 30 : strlen(ip));
    	}
	    memcpy(run_argv->port, port, strlen(port) > 6 ? 6 : strlen(port));
	}
	else
	{
		parse_regargs(argc, argv, run_argv);
	}
	
	if(strcmp(run_argv->deamon, "no")){
	   daemonize(sa_handler_SIGHUP);
    }
    
	init_trace_ex(run_argv->config, "/proxy/log/proxy");
	dbgmsg("proxy_init is run....");
	if(-1 == sys_conf_init(run_argv->config, &sys_conf))
	{
		fatal_quit_f("proxy conf init error!");
		return -1;
	}
	dbgmsg("proxy_init init 2....");
    //初始化编码转化
	if (-1 == init_encode_converter()) {
		return -1;
	}
    dbgmsg("proxy_init init 3....");
	if(already_running())
	{
		fatal_quit_f("proxy daemon already running!");
	}
	
	rl_file.rlim_cur = rl_file.rlim_max = 2 * 8192;
	if(0 != setrlimit(RLIMIT_NOFILE, &rl_file))
	{
		fatalmsg("setrlimit failed.");
	}
	rl_proc.rlim_cur = rl_proc.rlim_max = RLIM_INFINITY;
	if(0 != setrlimit(RLIMIT_NPROC, &rl_proc))
	{
		fatalmsg("setrlimit failed.");
	}

	errno = 0;
	getrlimit(RLIMIT_NOFILE, &rl_file);
	getrlimit(RLIMIT_NPROC, &rl_proc);
	dbgmsg("proxy_init init was over....");
	
	//启动myql数据库连接账号cache线程
	return 0;
}

static void* init_session(SOCKET cs, session_t *session)
{
    socklen_t rsa_len = sizeof(struct sockaddr_in);
    struct sockaddr_in rsa;
    char *ip = NULL;
    int port = 0;
    char guid[34] = {0};
    
    if(getpeername(cs, (struct sockaddr *)&rsa, &rsa_len) == 0)
    {
       ip = inet_ntoa(rsa.sin_addr);
       port = ntohs(rsa.sin_port);
       
       strcpy(session->sip, ip);
       session->sport = port;
    }
    
    if(getsockname(cs, (struct sockaddr *)&rsa, &rsa_len) == 0)
    {
        ip = inet_ntoa(rsa.sin_addr);
        strcpy(session->proxy_id, ip);
        strcpy(session->proxy_ip, ip);
    }
    
    random_uuid(guid);
    strcpy(session->session_id, guid);
    return 0;
}
/*
static void* proxy_change_title(char *title)
{
    int i = 0;
    int size = 0;
    for(i=0; i<rds_proxy_argc; i++)
    {
        size += strlen(rds_proxy_argv[i])+1;   
    }
    
    if(strlen(title) >= size){
        memcpy(rds_proxy_argv[0], title, size-1);
        rds_proxy_argv[0][size-1] = '\0';
    } else {
        memcpy(rds_proxy_argv[0], title, strlen(title));
        for(i = strlen(title); i< size;i++){
            rds_proxy_argv[0][i] = '\0';
        }
    }
    
    
//    memcpy(rds_proxy_argv[0],"proxy childen dfdsaf", sizeof("proxy childen dfdsaf"));
//    rds_proxy_argv[0][sizeof("proxy childen dfdsaf")] = '\0';
    return 0;    
}
*/
static void* proxy_thread_main(void *arg)
{
    static volatile int thr_cnt = 0;
	SOCKET cs = *(SOCKET *)arg;
	char buf[128];
	int len = 0;
    struct timeval		tval;
    parser_proto *parser_p = NULL;
    session_t session;
    char title[50] = {0};
    char *control_type = sys_conf.control.type;
    char *control_protocol = sys_conf.control.protocol;
    
    thr_cnt++;
    dbgmsg("proxy thread begin, thread count: %d.", thr_cnt);
    
    memset(&session, 0, sizeof(session_t));
    init_session(cs, &session);
    
    sprintf(title, "rdsproxy-%s %s", control_type, session.session_id);
    
    //proxy_change_title(title);
    pid_change_title(rds_proxy_argc, rds_proxy_argv, title);
    
    dbgmsg("session:%s,%s,%d",session.session_id,session.sip, session.sport);
    bzero(&tval, sizeof(tval));
	tval.tv_sec = 120; //recv and send timeout value
	tval.tv_usec = 0;

    setsockopt(cs, SOL_SOCKET, SO_RCVTIMEO, (void*)&tval, sizeof(tval));
    
    //分析协议
    parser_p = get_parser_proto_by_raw(control_protocol, buf, len);
    //调用处理器
    if(parser_p)
	{
		dbgmsg("protocol name=%s", parser_p->proto_name);
		parser_p->process(&session, cs, buf, len);
	}
	else
	{
		fatalmsg("protocol is not repport");
		goto err;
	}
    
err:
	thr_cnt--;
	fatalmsg("session was over");
//exit:
	if (isvalidsock(cs))
	{
	    CLOSE_SOCKET(cs);
		cs = -1;
	}
	return (void*)0;	    
}


static void proxy_proc(const char *ip, const char *port)
{
	SOCKET as = -1;
	pid_t child_pid;
	
	if(!port) port = "80";
    //启动代理协议cache线程
    if(strcmp(sys_conf.control.protocol, "mysql")==0 && start_mysql_cache_thread()){
        return;
    }
    if(strcmp(sys_conf.control.protocol, "redis")==0 && start_redis_cache_thread()){
        return;
    }
    
#ifdef __PROCESS__
	signal(SIGCHLD, SIG_IGN);
#endif
    dbgmsg("ip=%s,port=%s",ip, port);
	listen_socket = tcp_server(ip, port, fatal_quit_f);
	if (!isvalidsock(listen_socket))
    {
		return;
	}
	
	//recv SIGHUP
	while(isvalidsock(listen_socket))
	{
		as = accept(listen_socket, NULL, NULL);
		if(isvalidsock(as))
		{
#ifdef __PROCESS__
			child_pid = fork();
			if(0 < child_pid) 
			{
				CLOSE_SOCKET(as);
				as = -1;
			} 
			else if(0 == child_pid) 
			{
				CLOSE_SOCKET(listen_socket);
				listen_socket = -1;
				dbgmsg("proxy_thread_main %d",as);
				//#include <sys/prctl.h> //进程改名称
				//prctl(PR_SET_NAME,"Child",NULL,NULL,NULL); 
				proxy_thread_main(&as);
				
				exit(0);
			} 
			else 
			{
				CLOSE_SOCKET(as);
				as = -1;
				fatalmsg("fork failed.");
			}
#else
			if(launch_thread(proxy_thread_main, &as, 0, 32 * 1024, NULL))
			{
				CLOSE_SOCKET(as);
				as = -1;
			}

			usleep(40);//let new thread run and avoid some problem when limit the max open file
#endif
		}
		else
		{
			if(EINTR == errno) continue;
			if(!isvalidsock(as)){
				fatalmsg("accept failed.");
				sleep(1);
			}
		}
	}
}

int main(int argc, char **argv)
{
	char *ip   = NULL;
	char *port = NULL;
	int32_t i = 0;
    argv_conf run_argv;
    char guid[34];
	for (i = 3; i < 65535; i++) 
	{
		close(i);
	}
	
	rds_proxy_argc = argc;
	rds_proxy_argv = argv;
	
	random_uuid(guid);
    memset(&run_argv, 0, sizeof(argv_conf));
    
    strcpy(run_argv.config, "./proxy.xml");
    strcpy(run_argv.port, "80");
    
	if (-1 == proxy_init(argc, argv, &run_argv)) 
	{
		return 0;
	}
	
//	if ((3 == argc) && ('-' != argv[1][0]))
//	{
//		ip = strcasecmp("ANY", argv[1])? argv[1] : NULL;
//		port = argv[2];
//	}
    dbgmsg("proxy ip=%s,port=%s, config=%s", run_argv.ip, run_argv.port, run_argv.config);
	//port = argv[2];
	dbgmsg("proxy_thread_main %s,%s",run_argv.ip, run_argv.port);
	
	while(1) 
	{
	    if (strlen(run_argv.ip)>0){
	        ip = run_argv.ip;
	    }
	    port = run_argv.port;
		proxy_proc(ip, run_argv.port);
	}
	return 0;
}

