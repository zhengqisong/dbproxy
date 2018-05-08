#include "etcp.h"
#include "../trace/trace.h"

/* set_address - fill in a sockaddr_in structure */
void set_address( const char *hname, const char *sname,
	struct sockaddr_in *sap, const char *protocol )
{
	struct servent *sp;
	struct hostent *hp;
	char *endptr;
	short port;

	bzero( sap, sizeof( *sap ) );
	sap->sin_family = AF_INET;
	if ( hname != NULL ) {
		if ( !inet_aton( hname, &sap->sin_addr ) )
		{
			hp = gethostbyname( hname );
			if ( hp == NULL ) {
				fatalmsg( "unknown host: %s", hname );
			} else {
				sap->sin_addr = *( struct in_addr * )hp->h_addr;
			}
		}
	} else {
		sap->sin_addr.s_addr = htonl( INADDR_ANY );
	}
	port = strtol( sname, &endptr, 0 );
	if (port < 1024) {
//		fatalmsg("set address port < 1024.");	
//		return ;
	}
	if ( *endptr == '\0' ){
		sap->sin_port = htons( port );
	}
	else
	{
		sp = getservbyname( sname, protocol );
		if ( sp == NULL ) {
			fatalmsg( "unknown service: %s", sname );
		} else {
			sap->sin_port = sp->s_port;
		}
	}
}
