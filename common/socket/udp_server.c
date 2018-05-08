#include "etcp.h"

/* udp_server - set up a UDP server */
SOCKET udp_server( const char *hname, const char *sname,
			void(*errfun)(const char*, ...) )
{
	SOCKET s;
	struct sockaddr_in local;

	set_address( hname, sname, &local, "udp" );
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( s ) )
		if( errfun ) errfun( "socket call failed." );
	if ( bind( s, ( struct sockaddr * ) &local,
		 sizeof( local ) ) ){
		if( errfun ) errfun( "bind failed." );
		return -1;
	}

	return s;
}
