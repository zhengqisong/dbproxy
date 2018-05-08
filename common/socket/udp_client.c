#include "etcp.h"

/* udp_client - set up a UDP client */
SOCKET udp_client( const char *hname, const char *sname,
	struct sockaddr_in *sap, void(*errfun)(const char*, ...) )
{
	SOCKET s;

	set_address( hname, sname, sap, "udp" );
	s = socket( AF_INET, SOCK_DGRAM, 0 );
	if ( !isvalidsock( s ) ){
		if( errfun ) errfun( "socket call failed." );
		return -1;
	}

	return s;
}
