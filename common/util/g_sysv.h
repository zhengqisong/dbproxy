#ifndef _G_SYSV_MYH_20110124_83E29D1DE3EB__INCLUDED_
#define _G_SYSV_MYH_20110124_83E29D1DE3EB__INCLUDED_

const char* get_exefpath();
const char* get_exepath();
const char* get_exename();

int launch_thread(void*(*start_rtn)(void*), void *arg,
	       int joinable, int stacksize, pthread_t *tid);
int already_running();

typedef void Sigfunc(int);
Sigfunc *signal(int, Sigfunc *);
void daemonize(Sigfunc);

void* pid_change_title(int main_argc, char **main_argv, char *title);
#endif
