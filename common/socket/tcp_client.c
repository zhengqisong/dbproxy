#include "etcp.h"
#include "../trace/trace.h"

/* tcp_client - set up for a TCP client */
SOCKET tcp_client( const char *hname, const char *sname,
		int delayack, void(*errfun)(const char*, ...) )
{
	struct sockaddr_in peer;
	SOCKET s;

	set_address( hname, sname, &peer, "tcp" );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( !isvalidsock( s ) ){
		//if( errfun ) errfun( "socket call failed." );
	}else{
		if(delayack){
			int optval = 0;
			setsockopt(s, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
		}
		if ( connect( s, ( struct sockaddr * )&peer, sizeof( peer ) ) ){
			CLOSE_SOCKET(s);
			//if( errfun ) errfun( "connect failed." );
			return -1;
		}
	}

	return s;
}
