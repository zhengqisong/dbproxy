#ifndef __SKEL_H__
#define __SKEL_H__

/* UNIX version */

#define INIT_SOCKET()			
#define CLOSE_SOCKET(s)		if ( close( s ) ) fatalmsg( "close failed." )
#define set_errno(e)	errno = ( e )
#define isvalidsock(s)	( ( s ) >= 0 )
#define out_uint32_le(s, v) \
{ \
	  *(s) = (unsigned char)(v); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 8); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 16); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 24); \
	  (s)=(s)+1; \
}

#define out_uint32_be(s, v) \
{ \
	  *(s) = (unsigned char)((v) >> 24); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 16); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 8); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)(v); \
	  (s)=(s)+1; \
}

#define out_uint16_le(s, v) \
{ \
	  *(s) = (unsigned char)(v); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)((v) >> 8); \
	  (s)=(s)+1; \
}

#define out_uint16_be(s, v) \
{ \
	  *(s) = (unsigned char)((v) >> 8); \
	  (s)=(s)+1; \
	  *(s) = (unsigned char)(v); \
	  (s)=(s)+1; \
}

#define out_uint8a(s, v, n) \
{ \
		memcpy((s), (v), (n)); \
}

#define in_uint16_le(s, v) \
{ \
	(v) = (unsigned short) \
    ( \
      (*((unsigned char*)(s)) << 0) | \
      (*((unsigned char*)((s) + 1)) << 8) \
    ); \
  (s) += 2; \
}

#define in_uint16_be(s, v) \
{ \
  (v) = *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
}

#define in_uint32_le(s, v) \
{ \
	(v) = (unsigned int) \
    ( \
      (*((unsigned char*)(s)) << 0) | \
      (*((unsigned char*)((s) + 1)) << 8) |\
      (*((unsigned char*)((s) + 2)) << 16) |\
      (*((unsigned char*)((s) + 3)) << 24) \
    ); \
  (s) += 4; \
}

#define in_uint32_be(s, v) \
{ \
  (v) = *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 24; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 16; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
}

#define in_uint24_le(s, v) \
{ \
	(v) = (unsigned int) \
    ( \
      (*((unsigned char*)(s)) << 0) | \
      (*((unsigned char*)((s) + 1)) << 8) |\
      (*((unsigned char*)((s) + 2)) << 16) \
    ); \
  (s) += 3; \
}

#define in_uint24_be(s, v) \
{ \
  (v) = *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 16; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
  (v) <<= 8; \
  (v) |= *((unsigned char*)(s)); \
  (s) =(s) + 1; \
}

typedef int SOCKET;

#endif  /* __SKEL_H__ */
