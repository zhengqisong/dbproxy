#ifndef _RESPONSOR_SESSION_H
#define _RESPONSOR_SESSION_H


enum {
	SESSION_AUTH_ERR,
	SESSION_AUTH_SUC,
	SESSION_RUN,
	SESSION_STOP,
	SESSION_INTERFACE_ERR,
	SESSION_TCP_ERR,
	SESSION_LOCK_ERR,
	SESSION_OTHER_ERR
} ;

typedef struct _session_t{
    char session_id[64];
    char proxy_id[32];
    char proxy_ip[32];
    char sip[32];
    char dip[32];
    unsigned short sport;
    unsigned short dport;
    char account[128];
    
    int status;
    char instance_name[64];
    char dbname[64];
} session_t;

int send_auth_message(session_t *session, char *message);
int send_logout_message(session_t *session, char *message);
int send_audit_message(session_t *session, char *result, float spend_time, char *sql, char *message);
#endif
