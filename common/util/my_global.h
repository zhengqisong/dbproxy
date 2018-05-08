#ifndef _global_h
#define _global_h

#define SIZEOF_INT 4
#define SIZEOF_LONG 8

#ifndef HAVE_UCHAR
typedef unsigned char	uchar;	/* Short for unsigned char */
#endif

#ifndef HAVE_INT8
typedef signed char int8;       /* Signed integer >= 8  bits */
#endif
#ifndef HAVE_UINT8
typedef unsigned char uint8;    /* Unsigned integer >= 8  bits */
#endif
#ifndef HAVE_INT16
typedef short int16;
#endif
#ifndef HAVE_UINT16
typedef unsigned short uint16;
#endif

#if SIZEOF_INT == 4

#ifndef HAVE_INT32
typedef int int32;
#endif
#ifndef HAVE_UINT32
typedef unsigned int uint32;
#endif

#elif SIZEOF_LONG == 4

#ifndef HAVE_INT32
typedef long int32;
#endif

#ifndef HAVE_UINT32
typedef unsigned long uint32;
#endif

#else
#error Neither int or long is of 4 bytes width
#endif

#if !defined(HAVE_ULONG) && !defined(__USE_MISC)
typedef unsigned long	ulong;		  /* Short for unsigned long */
#endif
#ifndef longlong_defined
/* 
  Using [unsigned] long long is preferable as [u]longlong because we use 
  [unsigned] long long unconditionally in many places, 
  for example in constants with [U]LL suffix.
*/
#if defined(HAVE_LONG_LONG) && SIZEOF_LONG_LONG == 8
typedef unsigned long long int ulonglong; /* ulong or unsigned long long */
typedef long long int	longlong;
#else
typedef unsigned long	ulonglong;	  /* ulong or unsigned long long */
typedef long		longlong;
#endif
#endif
#ifndef HAVE_INT64
typedef longlong int64;
#endif
#ifndef HAVE_UINT64
typedef ulonglong uint64;
#endif

#define SCRAMBLE_LENGTH 20

#endif /* my_global_h */
