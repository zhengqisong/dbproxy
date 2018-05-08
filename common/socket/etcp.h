#ifndef __ETCP_H__
#define __ETCP_H__

/* Include standard headers */

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/ioctl.h> 
#include <stdarg.h>
#include <string.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "skel.h"

#define TRUE			1
#define FALSE			0
#define NLISTEN			5		/* max waiting connections */
#define NSMB			5		/* number shared memory bufs */
#define SMBUFSZ			256		/* size of shared memory buf */


#ifdef __SVR4
#define bzero(b,n)	memset( ( b ), 0, ( n ) )
#endif 

typedef void ( *tofunc_t )( void * );

int readn( SOCKET, char *, size_t );
int readvrec( SOCKET, char *, size_t );
int readcrlf( SOCKET, char *, size_t );
int readline( SOCKET, char *, size_t );
int tcp_server( const char *, const char *, void(*)(const char*, ...) );
SOCKET tcp_client( const char *hname, const char *sname, int delayack, void(*errfun)(const char*, ...) );
int tcp_client_ex( unsigned int ip, short port, int delayack, void(*errfun)(const char*, ...) );
int udp_server( const char *, const char *, void(*)(const char*, ...) );
int udp_client( const char *, const char *, struct sockaddr_in *, void(*)(const char*, ...) );
int tselect( int, fd_set *, fd_set *, fd_set *);
unsigned int timeout( tofunc_t, void *, int );
void untimeout( unsigned int );
void init_smb( int );
void *smballoc( void );
void smbfree( void * );
void smbsend( SOCKET, void * );
void *smbrecv( SOCKET );
void set_address( const char *, const char *, struct sockaddr_in *, const char * );
int sendvrec( SOCKET fd, char *bp, size_t len );
int tcp_read( SOCKET fd, char *bp, int len );
int tcp_send( SOCKET fd, char *bp, int len );
int send_udp_msg(void * msg, int msg_size, char * szaddr, int port);
#endif  /* __ETCP_H__ */
