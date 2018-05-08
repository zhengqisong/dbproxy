#include "etcp.h"

/* tcp_server - set up for a TCP server */
SOCKET tcp_server( const char *hname, const char *sname,
			void(*errfun)(const char*, ...) )
{
	struct sockaddr_in local;
	SOCKET s;
	const int on = 1;

	set_address( hname, sname, &local, "tcp" );
	s = socket( AF_INET, SOCK_STREAM, 0 );
	if ( !isvalidsock( s ) ) {
		if( errfun ) {
			errfun( "socket call failed." );
		}
		goto err;
	}
	if ( setsockopt( s, SOL_SOCKET, SO_REUSEADDR,( char * )&on, sizeof( on ) ) ) {
		if( errfun ) {
			errfun( "setsockopt failed." );
		}
		goto err;
	}

	if ( bind( s, ( struct sockaddr * ) &local, sizeof( local ) ) ) {
		if( errfun ) {
			errfun( "bind failed." );
		}
		goto err;
	}

	if ( listen( s, NLISTEN ) ) {
		if( errfun ) {
			errfun( "listen failed." );
		}
		goto err;
	}

	return s;
err:
	if (isvalidsock(s)) {
		close(s);
		s = -1;
	}	

	return -1;
}
