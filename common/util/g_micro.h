#ifndef _G_MICRO_MYH_E1897E85_F5B9_4FC6_A1B1_83E29D1DE3EB__INCLUDED_
#define _G_MICRO_MYH_E1897E85_F5B9_4FC6_A1B1_83E29D1DE3EB__INCLUDED_


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef offsetof
#define offsetof(s,m)       (size_t)&(((s *)0)->m)
#endif

#define DIMOF(a)            (sizeof((a)) / sizeof((a[0])))


#define EXIT(s)              exit(s)
#define THREAD_EXIT(s)       pthread_exit((void*)s)

#define MAX_PATH             256

#endif
