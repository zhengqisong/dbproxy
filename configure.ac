#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.59)
AC_INIT(proxy.c,1.0,[zqs@sohu.com])

AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB

# Checks for libraries.
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB(m, main)
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB(pthread, main)
# FIXME: Replace `main' with a function in `-lz':
AC_CHECK_LIB(z, main)

# Checks for header files.
AC_CHECK_HEADERS(arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/ioctl.h sys/socket.h sys/time.h unistd.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS(bzero ftruncate gethostbyname gettimeofday inet_ntoa memchr memset socket strcasecmp strchr strerror strncasecmp strrchr strstr strtol)

AC_CONFIG_FILES(Makefile
                 common/Makefile
                 common/conf/Makefile
                 common/socket/Makefile
                 common/trace/Makefile
                 common/util/Makefile
                 responsor/Makefile
                 processor/Makefile
                 proxy/Makefile)
AC_OUTPUT
