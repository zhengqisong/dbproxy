#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include "trace.h"
#include "g_micro.h"
#include "g_sysv.h"
#include "g_string.h"

const char* get_exefpath(){
	static char name[MAX_PATH] = {0};

	if(!name[0]){
		char proc_path[32];
		int rllen = 0;

		sprintf(proc_path, "/proc/%d/exe", getpid());
		rllen = readlink(proc_path, name, sizeof(name));
		if(-1 == rllen)
		{
			*name = 0;
		}
		name[min(rllen, sizeof(name) - 1)] = 0;
	}

	return name;
}

const char* get_exepath(){
	static char name[MAX_PATH] = {0};
	
	if(!name[0]){
		const char *path = get_exefpath();
		if(*path){
		       	strcpy(name, path);
			*strrchr(name, '/') = 0;
		}
	}

	return name;
}

const char* get_exename(){
	char *pos = NULL;
	static char name[MAX_PATH] = {0};
	
	if(!name[0]){
		const char *path = get_exefpath();
		pos = strrchr(path, '/');
		if (NULL == pos) {
			goto err;	
		}
		if (*path) {
			v_strncpy(name, sizeof(name), pos + 1, strlen(pos + 1));
		}
	}
err:
	return name;
}

//<<Advanced Programming in the UNIX Environment - Second Edition>>P.343
void daemonize(Sigfunc sighup_hdlr){
	const char *     cmd = get_exename();
	int              i;
	int		fd0, fd1, fd2;
	pid_t            pid;
//	struct rlimit    rl;

	umask(0);

#if 0
	if(getrlimit(RLIMIT_NOFILE, &rl) < 0)
		fatal_quit_f("%s: can't get file limit", cmd);
#endif	
	if((pid = fork()) < 0)
		fatal_quit_f("%s: can't fork", cmd);
	else if (pid != 0)
		exit(0);
	setsid();

	if(signal(SIGHUP, sighup_hdlr? sighup_hdlr : SIG_IGN) < 0)
		fatal_quit_f("%s: can't ignore SIGHUP", cmd);
	if((pid = fork()) < 0)
		fatal_quit_f("%s: can't fork", cmd);
	else if(pid != 0)
		exit(0);
#if 0	
	if(chdir("/") < 0)
		fatal_quit_f("%s: can't change directory to /", cmd);

	if(rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;

#endif

	for(i = 0; i < 3; i++) {
		close(i);
	}
	
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	//openlog(cmd, LOG_CONS, LOG_DAEMON);
	if(fd0 != 0 || fd1 != 1 || fd2 != 2){
		//syslog(LOG_ERR, "unexpected file descriptors %d %d %d", fd0, fd1, fd2);
		fatal_quit_f("unexpected file descriptors %d %d %d", fd0, fd1, fd2);
	}
}

//<<Advanced Programming in the UNIX Environment - Second Edition>>P.315
int launch_thread(void*(*start_rtn)(void*), void *arg,
	       	int joinable, int stacksize, pthread_t *tid){
	int            err;
	pthread_attr_t attr;
	pthread_t my_tid;
	err = pthread_attr_init(&attr);
	if(0 != err){
		fatalmsg("pthread_attr_init() failed. errno=%d(%d)", err, errno);
		return err;
	}
	
	if(0 != (err = pthread_attr_setdetachstate(&attr,
				 joinable? PTHREAD_CREATE_JOINABLE : PTHREAD_CREATE_DETACHED))){
		fatalmsg("pthread_attr_setdetachstate() failed. errno=%d(%d)", err, errno);
	}
	if(stacksize && (0 != (err = pthread_attr_setstacksize(&attr, stacksize)))){
		fatalmsg("pthread_attr_setstacksize() failed. errno=%d(%d)", err, errno);
	}
	if(0 == err){
		err = pthread_create(tid? tid : &my_tid, &attr, start_rtn, arg);
	}
	pthread_attr_destroy(&attr);
	if(0 != err){
		fatalmsg("pthread_create() failed. errno=%d(%d)", err, errno);
	}

	return err;
}

//<<Advanced Programming in the UNIX Environment - Second Edition>>P.349
int already_running(){
	int fd = 0;
	char lockfile[2 * MAX_PATH];

	snprintf(lockfile, sizeof(lockfile), "%s/.%s.lock", get_exepath(), get_exename());
	fd = open(lockfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(0 < fd){
		return lockf(fd, F_TLOCK, 0);
	}
	fatalmsg("cannot open singleton lock file.");
	return 1;
}

//<<Advanced Programming in the UNIX Environment - Second Edition>>P.265
Sigfunc *signal(int signo, Sigfunc *func){
	struct sigaction act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(signo == SIGALRM){
#ifdef SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;
#endif
	}else{
#ifdef SA_RESTART
		act.sa_flags |= SA_RESTART;
#endif
	}
	if(sigaction(signo, &act, &oact)  < 0)
		return (SIG_ERR);
	return (oact.sa_handler);
}

void* pid_change_title(int main_argc, char **main_argv, char *title)
{
    int i = 0;
    int size = 0;
    for(i=0; i<main_argc; i++)
    {
        size += strlen(main_argv[i])+1;   
    }
    
    if(strlen(title) >= size){
        memcpy(main_argv[0], title, size-1);
        main_argv[0][size-1] = '\0';
    } else {
        memcpy(main_argv[0], title, strlen(title));
        for(i = strlen(title); i< size;i++){
            main_argv[0][i] = '\0';
        }
    }
    
    
//    memcpy(rds_proxy_argv[0],"proxy childen dfdsaf", sizeof("proxy childen dfdsaf"));
//    rds_proxy_argv[0][sizeof("proxy childen dfdsaf")] = '\0';
    return 0;    
}