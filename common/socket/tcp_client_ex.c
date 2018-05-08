#include "etcp.h"
#include "../trace/trace.h"

/* tcp_client - set up for a TCP client */
SOCKET tcp_client_ex( unsigned int ip, short port, int delayack,
			void(*errfun)(const char*, ...) )
{
	struct sockaddr_in peer;
	SOCKET s;

	bzero( &peer, sizeof( peer ) );
	peer.sin_family = AF_INET;
	peer.sin_addr.s_addr = htonl( ip );
	peer.sin_port = htons( port );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( !isvalidsock( s ) ){
		if( errfun ) errfun("socket call failed." );
	}else{
		if(delayack){
			int optval = 0;
			setsockopt(s, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval));
		}
		if ( connect( s, ( struct sockaddr * )&peer, sizeof( peer ) ) ){
			CLOSE_SOCKET(s);
			if( errfun ) errfun( "connect failed." );
			return -1;
		}
	}


	return s;
}
