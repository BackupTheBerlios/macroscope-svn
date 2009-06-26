/*-
 * Copyright 2009 Guram Dukashvili
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//---------------------------------------------------------------------------
#include "config.h"
//---------------------------------------------------------------------------
#ifndef kbootH
#define kbootH
//---------------------------------------------------------------------------
#if building_dll
# define DLIMPORT __declspec(dllexport)
#else
# if linking_to_dll
#  define DLIMPORT __declspec(dllimport)
# else
#  define DLIMPORT
# endif
#endif
//---------------------------------------------------------------------------

#ifdef __CYGWIN__
#ifdef __WIN32__
#undef __WIN32__
#endif
#ifdef __WIN64__
#undef __WIN64__
#endif
#endif

#ifdef __linux__
#define _BSD_SOURCE 1
#define _XOPEN_SOURCE 600
#endif

#if HAVE_STDAFX_H
#include <stdafx.h>
#endif

#if HAVE_STDDEF_H
#include <stddef.h>
#endif

#if HAVE_STDINT_H
#ifndef __STDC_CONSTANT_MACROS // for freebsd
#define __STDC_CONSTANT_MACROS 1
#endif
#ifndef __STDC_LIMIT_MACROS // for freebsd
#define __STDC_LIMIT_MACROS 1
#endif
#ifndef __STDC_FORMAT_MACROS // for linux
#define __STDC_FORMAT_MACROS
#endif
#include <stdint.h>
#endif

#if HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#if HAVE_MACHINE_ATOMIC_H
#include <machine/atomic.h>
#endif

#if HAVE_MACHINE_CPUFUNC_H
#include <machine/cpufunc.h>
#endif

#if HAVE_MACHINE_SPECIALREG_H
#include <machine/specialreg.h>
#endif

#if HAVE_MEMORY_H
#include <memory.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_ASSERT_H
#if __BCPLUSPLUS__ && !defined(_DEBUG) && !defined(NDEBUG)
#define NDEBUG 1
#endif
#include <assert.h>
#endif

#if HAVE_STRING_H
#include <string.h>
#endif

#if HAVE_ERR_H
#include <err.h>
#endif

#if HAVE_ERRNO_H
#include <errno.h>
#endif

#if SIZEOF_ULONG == 0
typedef unsigned long ulong;
#undef SIZEOF_ULONG
#define SIZEOF_ULONG sizeof(ulong)
#endif

#if SIZEOF_WCHAR_T == 0
typedef short wchar_t;
#undef SIZEOF_WCHAR_T
#define SIZEOF_WCHAR_T sizeof(short)
#endif

#if !defined(PACKED) && defined(__GNUG__)
#define PACKED __attribute__ ((packed))
#endif

#if SIZEOF_CHAR_T != 1
#error size of char != 1
#endif

#if SIZEOF_INT8_T == 0
#if SIZEOF_CHAR_T == 1
typedef signed char int8_t;
typedef unsigned char uint8_t;
#elif SIZEOF__INT8 == 1
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
#endif
#undef SIZEOF_INT8_T
#define SIZEOF_INT8_T 1
#endif

#if SIZEOF_INT16_T == 0
#if SIZEOF__INT16 == 2
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
#elif SIZEOF_SHORT == 2
typedef short int16_t;
typedef unsigned short uint16_t;
#else
#error sizeof(short) != 2
#endif
#undef SIZEOF_INT16_T
#define SIZEOF_INT16_T 2
#endif

#if SIZEOF_INT32_T == 0
#if SIZEOF__INT32 == 4
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
#define HAVE_INT32_T_AS_INT 1
#define HAVE_INT_AS_INT32_T 1
#elif SIZEOF_INT == 4
typedef int int32_t;
typedef unsigned int uint32_t;
#define HAVE_INT32_T_AS_INT 1
#define HAVE_INT_AS_INT32_T 1
#else
#error sizeof(int) != 4
#endif
#undef SIZEOF_INT32_T
#define SIZEOF_INT32_T 4
#if (_MSC_VER || _WIN32) && SIZEOF_INTPTR_T != 0
#define HAVE_INT32_T_AS_INTPTR_T (SIZEOF_INT32_T == SIZEOF_VOID_P)
#define HAVE_INTPTR_T_AS_INT32_T HAVE_INT32_T_AS_INTPTR_T
#endif
#endif

#if SIZEOF_INT64_T == 0
#if SIZEOF__INT64 == 8
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#elif SIZEOF_LONG_LONG == 8
typedef long long int64_t;
typedef unsigned long long uint64_t;
#elif SIZEOF_LONG_INT == 8
typedef long int int64_t;
typedef unsigned long int uint64_t;
#elif SIZEOF_LONG == 8
typedef long int64_t;
typedef unsigned long uint64_t;
#else
#error System not support 64 bit integer type 
#endif
#undef SIZEOF_INT64_T
#define SIZEOF_INT64_T 8
#if (_MSC_VER || _WIN32) && SIZEOF_INTPTR_T != 0
#define HAVE_INT64_T_AS_INTPTR_T (SIZEOF_INT64_T == SIZEOF_VOID_P)
#define HAVE_INTPTR_T_AS_INT64_T HAVE_INT64_T_AS_INTPTR_T
#endif
#endif

#if SIZEOF_INTMAX_T == 0
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#undef SIZEOF_INTMAX_T
#define SIZEOF_INTMAX_T 8
#define HAVE_INTMAX_T_AS_INT64_T 1
#define HAVE_INT64_T_AS_INTMAX_T 1
#if SIZEOF_INTPTR_T == SIZEOF_INTMAX_T
#define HAVE_INTPTR_T_AS_INTMAX_T 1
#define HAVE_MAXPTR_T_AS_INTPTR_T 1
#endif
#if SIZEOF_INTPTR_T == 8
#ifndef HAVE_INTPTR_T_AS_INT64_T
#define HAVE_INTPTR_T_AS_INT64_T 1
#endif
#ifndef HAVE_INT64_T_AS_INTPTR_T
#define HAVE_INT64_T_AS_INTPTR_T 1
#endif
#define HAVE_INTMAX_T_AS_INTPTR_T 1
#endif
#endif

#if SIZEOF_INTPTR_T == 0
#undef SIZEOF_INTPTR_T
#if SIZEOF_INT == SIZEOF_VOID_P
typedef int intptr_t;
typedef unsigned int uintptr_t;
#define SIZEOF_INTPTR_T 4
#define HAVE_INTPTR_T_AS_INT 1
#define HAVE_INTPTR_T_AS_INT32_T HAVE_INT32_T_AS_INT
#define HAVE_INT32_T_AS_INTPTR_T HAVE_INT32_T_AS_INT
#elif SIZEOF_INT32_T == SIZEOF_VOID_P
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#define SIZEOF_INTPTR_T 4
#define HAVE_INTPTR_T_AS_INT32_T 1
#define HAVE_INT32_T_AS_INTPTR_T 1
#elif SIZEOF_INT64_T == SIZEOF_VOID_P
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;
#define SIZEOF_INTPTR_T 8
#define HAVE_INTPTR_T_AS_INT64_T 1
#define HAVE_INT64_T_AS_INTPTR_T 1
#define HAVE_INTPTR_T_AS_INTMAX_T 1
#define HAVE_INTMAX_T_AS_INTPTR_T 1
#endif
#endif

#if SIZEOF_PTRDIFF_T == 0
#undef SIZEOF_PTRDIFF_T
#if SIZEOF_INT32_T == SIZEOF_VOID_P
typedef int32_t ptrdiff_t;
#define SIZEOF_PTRDIFF_T 4
#elif SIZEOF_INT64_t == SIZEOF_VOID_P
typedef int64_t ptrdiff_t;
#define SIZEOF_PTRDIFF_T 8
#endif
#endif

#if SIZEOF_LONG_DOUBLE > 0 && SIZEOF_LONG_DOUBLE > SIZEOF_DOUBLE
#define HAVE_LONG_DOUBLE 1
typedef long double ldouble;
#else
typedef double ldouble;
#endif

#ifndef __XSTRING
#define __STRING(x) #x
#define __XSTRING(x) __STRING(x)
#endif

#ifndef SIZEOF_PID_T
#define SIZEOF_PID_T 0
#endif

#if SIZEOF_PID_T == 0
typedef int32_t pid_t;
#undef SIZEOF_PID_T
#define SIZEOF_PID_T sizeof(int32_t)
#endif

#ifndef SIZEOF_UID_T
#define SIZEOF_UID_T 0
#endif

#if SIZEOF_UID_T == 0
typedef int32_t uid_t;
#undef SIZEOF_UID_T
#define SIZEOF_UID_T sizeof(int32_t)
#endif

#ifndef SIZEOF_GID_T
#define SIZEOF_GID_T 0
#endif

#if SIZEOF_GID_T == 0
typedef int32_t gid_t;
#undef SIZEOF_GID_T
#define SIZEOF_GID_T sizeof(int32_t)
#endif

#if SIZEOF_STAT64 > 0
#define HAVE_STAT64 1
#endif

#if SIZEOF__STAT64 > 0
#define HAVE__STAT64 1
#endif

#if (defined(_WIN32) || defined (WIN32)) && !defined(__WIN32__)
#define __WIN32__ 1
#endif

#if (defined(_WIN64) || defined (WIN64)) && !defined(__WIN64__)
#define __WIN64__ 1
#endif

#if !defined(_WINDOWS) && (defined(__WIN32__) || defined(__WIN64__))
#define _WINDOWS 1
#endif

#if defined(__WIN32__) || defined(__WIN64__)
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#if _WIN32_WINNT < 0x0502
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#ifndef INT8_C
#define INT8_C(c)               (c)
#define UINT8_C(c)              (c)
#define INT16_C(c)              (c)
#define UINT16_C(c)             (c)
#define INT32_C(c)              (c)
#define UINT32_C(c)             (c ## u)
#define INT64_C(c)              (c ## i64)
#define UINT64_C(c)             (c ## ui64)
#define INTMAX_C(c)             (c ## i64)
#define UINTMAX_C(c)            (c ## ui64)
#endif
#endif

#if defined(__WIN32__) && !defined(__WIN64__)
#ifndef INT8_MIN
#define INT8_MIN        (-0x7f-1)
#define INT16_MIN       (-0x7fff-1)
#define INT32_MIN       (-0x7fffffff-1)
#define INT64_MIN       (-0x7fffffffffffffffi64-1)
#define INT8_MAX        0x7f
#define INT16_MAX       0x7fff
#define INT32_MAX       0x7fffffff
#define INT64_MAX       0x7fffffffffffffffi64
#define UINT8_MAX       0xff
#define UINT16_MAX      0xffff
#define UINT32_MAX      0xffffffffi64
#define UINT64_MAX      0xffffffffffffffffui64

#define INTPTR_MIN      INT32_MIN
#define INTPTR_MAX      INT32_MAX
#define UINTPTR_MAX     UINT32_MAX
#define INTMAX_MIN      INT64_MIN
#define INTMAX_MAX      INT64_MAX
#define UINTMAX_MAX     UINT64_MAX
#define PTRDIFF_MIN     INT32_MIN
#define PTRDIFF_MAX     INT32_MAX
#ifndef SIZE_MAX
#define SIZE_MAX        UINT32_MAX
#endif
#define WINT_MIN        INT32_MIN
#define WINT_MAX        INT32_MAX
#endif

#define PRId8           "d"     /* int8_t */
#define PRId16          "d"     /* int16_t */
#define PRId32          "d"     /* int32_t */
#define PRId64          "I64d"  /* int64_t */
#define PRIdLEAST8      "d"     /* int_least8_t */
#define PRIdLEAST16     "d"     /* int_least16_t */
#define PRIdLEAST32     "d"     /* int_least32_t */
#define PRIdLEAST64     "I64d"  /* int_least64_t */
#define PRIdFAST8       "d"     /* int_fast8_t */
#define PRIdFAST16      "d"     /* int_fast16_t */
#define PRIdFAST32      "d"     /* int_fast32_t */
#define PRIdFAST64      "I64d"  /* int_fast64_t */
#define PRIdMAX         "I64d"  /* intmax_t */
#define PRIdPTR         "d"     /* intptr_t */

#define PRIu8           "u"     /* uint8_t */
#define PRIu16          "u"     /* uint16_t */
#define PRIu32          "u"     /* uint32_t */
#define PRIu64          "I64u"  /* uint64_t */
#define PRIuMAX         "I64u"  /* uintmax_t */
#define PRIuPTR         "u"     /* uintptr_t */

#define PRIx8           "x"     /* uint8_t */
#define PRIx16          "x"     /* uint16_t */
#define PRIx32          "x"     /* uint32_t */
#define PRIx64          "I64x"  /* uint64_t */
#define PRIxMAX         "I64x"  /* uintmax_t */
#define PRIxPTR         "x"     /* uintptr_t */

#define PRIX8           "X"     /* uint8_t */
#define PRIX16          "X"     /* uint16_t */
#define PRIX32          "X"     /* uint32_t */
#define PRIX64          "I64X"   /* uint64_t */
#define PRIXMAX         "I64X"    /* uintmax_t */
#define PRIXPTR         "X"     /* uintptr_t */
#endif

#ifdef __WIN64__
#define INT8_MIN        (-0x7f-1)
#define INT16_MIN       (-0x7fff-1)
#define INT32_MIN       (-0x7fffffff-1)
#define INT64_MIN       (-0x7fffffffffffffffI64-1)
#define INT8_MAX        0x7f
#define INT16_MAX       0x7fff
#define INT32_MAX       0x7fffffff
#define INT64_MAX       0x7fffffffffffffffL
#define UINT8_MAX       0xff
#define UINT16_MAX      0xffff
#define UINT32_MAX      0xffffffffI64
#define UINT64_MAX      0xffffffffffffffffUI64

#define INTPTR_MIN      INT64_MIN
#define INTPTR_MAX      INT64_MAX
#define UINTPTR_MAX     UINT64_MAX

#define INTMAX_MIN      INT64_MIN
#define INTMAX_MAX      INT64_MAX
#define UINTMAX_MAX     UINT64_MAX

#define PTRDIFF_MIN     INT64_MIN
#define PTRDIFF_MAX     INT64_MAX

#ifndef SIZE_MAX
#define SIZE_MAX        UINT64_MAX
#endif

#define WINT_MIN        INT32_MIN
#define WINT_MAX        INT32_MAX

#define PRId8           "d"     /* int8_t */
#define PRId16          "d"     /* int16_t */
#define PRId32          "d"     /* int32_t */
#define PRId64          "I64d"    /* int64_t */
#define PRIdMAX         "I64d"    /* intmax_t */
#define PRIdPTR         "I64d"    /* intptr_t */

#define PRIu8           "u"     /* uint8_t */
#define PRIu16          "u"     /* uint16_t */
#define PRIu32          "u"     /* uint32_t */
#define PRIu64          "I64u"    /* uint64_t */
#define PRIuMAX         "I64u"    /* uintmax_t */
#define PRIuPTR         "I64u"    /* uintptr_t */

#define PRIx8           "x"     /* uint8_t */
#define PRIx16          "x"     /* uint16_t */
#define PRIx32          "x"     /* uint32_t */
#define PRIx64          "I64x"    /* uint64_t */
#define PRIxMAX         "I64x"    /* uintmax_t */
#define PRIxPTR         "I64x"    /* uintptr_t */

#define PRIX8           "X"     /* uint8_t */
#define PRIX16          "X"     /* uint16_t */
#define PRIX32          "X"     /* uint32_t */
#define PRIX64          "I64X"    /* uint64_t */
#define PRIXMAX         "I64X"    /* uintmax_t */
#define PRIXPTR         "I64X"    /* uintptr_t */
#endif

#ifndef PRF_FLT
#define PRF_FLT ""
#endif

#ifndef PRF_DBL
#define PRF_DBL "l"
#endif

#ifndef PRF_LDBL
#if HAVE_LONG_DOUBLE
#define PRF_LDBL "L"
#else
#define PRF_LDBL PRF_DBL
#endif
#endif

#ifndef __GNUG__
#if !defined(__GNUG__) && _MSC_VER < 1300 && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ ""
#elif _MSC_VER >= 1300 && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif
#endif

#ifndef PACKED
#ifdef _MSC_VER
#define PACKED __declspec(align(1))
#elif __GNUC__
#define PACKED __attribute__ ((aligned (1)))
#else
#define PACKED
#endif
#endif
//---------------------------------------------------------------------------
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif

#if HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#if HAVE_STDARG_H
#include <stdarg.h>
#elif HAVE_VARARGS_H
#include <varargs.h>
#endif

#if HAVE_SYSEXITS_H
#include <sysexits.h>
#endif

#if HAVE_SYS_IPC_H
#include <sys/ipc.h>
#endif

#if HAVE_SYS_SEM_H
#include <sys/sem.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#if HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif

#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#if HAVE_NETINET_IN_SYSTM_H
#include <netinet/in_systm.h>
#endif

#if HAVE_NETINET_IP_H
#include <netinet/ip.h>
#endif

#if HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if HAVE_NET_IF_H
#include <net/if.h>
#endif

#if HAVE_NET_IF_TYPES_H
#include <net/if_types.h>
#endif

#if HAVE_NET_ROUTE_H
#include <net/route.h>
#endif

#if HAVE_NET_ETHERNET_H
#include <net/ethernet.h>
#endif

#if HAVE_NET_IF_DL_H
#include <net/if_dl.h>
#endif

#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#if HAVE_NETDB_H
#include <netdb.h>
#endif

#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif

#if HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif

#if HAVE_AIO_H
#include <aio.h>
#endif

#if HAVE_SYS_ENDIAN_H
#include <sys/endian.h>
#endif

#if HAVE_SHA256_H
#include <sha256.h>
#endif

#if HAVE_PROCESS_H
#include <process.h>
#endif

#if HAVE_DLFCN_H
#include <dlfcn.h>
#endif

#if HAVE_PTHREAD_H
#include <pthread.h>
#endif

#if HAVE_SIGNAL_H
#include <signal.h>
#ifndef _SIG_MAXSIG
#define _SIG_MAXSIG _NSIG
#endif
#endif

#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#if HAVE_STDIO_H
#include <stdio.h>
#endif

#if HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if HAVE_SEMAPHORE_H
#include <semaphore.h>
#endif

#if HAVE_CTYPE_H
#include <ctype.h>
#endif

#if HAVE_PWD_H
#include <pwd.h>
#endif

#if HAVE_GRP_H
#include <grp.h>
#endif

#if HAVE_IO_H
#include <io.h>
#endif

#if HAVE_PATHS_H
#include <paths.h>
#endif

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if HAVE_SYS_RTPRIO_H
#include <sys/rtprio.h>
#endif

#if HAVE_TIME_H
#include <time.h>
#endif

#if HAVE_SCHED_H
#include <sched.h>
#endif

#if HAVE_UTIME_H
#include <utime.h>
#endif

#if HAVE_SYS_UTIME_H
#include <sys/utime.h>
#endif

#if HAVE_SYS_TIMEB_H
#include <sys/timeb.h>
#endif

#if HAVE_SYS_UUID_H
#include <sys/uuid.h>
#endif

#if HAVE_UUID_H
#include <uuid.h>
#endif

#if HAVE_SYS_EVENT_H
#include <sys/event.h>
#endif

#if HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif

#if HAVE_SYS_SYSCTL_H
#include <sys/sysctl.h>
#endif

#if HAVE_SYS_EPOLL_H
#include <sys/epoll.h>
#endif

#if HAVE_MATH_H
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES 1
#endif
#include <math.h>
#endif

#if HAVE_FLOAT_H
#include <float.h>
#endif

#if HAVE_UCONTEXT_H
#include <ucontext.h>
#endif

#if HAVE_FAM_H
#include <fam.h>
#endif

// C++ headers
#ifdef __cplusplus

#if HAVE_TYPEINFO
#include <typeinfo>
#endif

#if HAVE_TYPEINFO_H
#include <typeinfo.h>
#endif

#if HAVE_STDEXEPT
#include <stdexcept>
#endif

#if HAVE_NEW
#include <new>
#elif HAVE_NEW_H
#include <new.h>
#endif

#endif // C++ headers

#ifdef __BORLANDC__
#pragma option push -w-eff
#pragma option push -w-sig
#pragma option push -w-aus
#endif

#if HAVE_WINSOCK2_H
#include <winsock2.h>
#elif HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_MSWSOCK_H || defined(__BORLANDC__)

#undef HAVE_MSWSOCK_H
#define HAVE_MSWSOCK_H 1

#ifdef __BORLANDC__
#ifndef _MSWSOCKDEF_
#pragma option push -b -a8 -pc -A- /*P_O_Push*/
#define _MSWSOCKDEF_

#if(_WIN32_WINNT >= 0x0600)
#ifdef _MSC_VER
#define MSWSOCKDEF_INLINE __inline
#else
#define MSWSOCKDEF_INLINE static inline
#endif
#endif //(_WIN32_WINNT>=0x0600)
#ifndef ASSERT
#define MSWSOCKDEF_ASSERT_UNDEFINED
#define ASSERT(exp) ((VOID) 0)
#endif
#ifdef __cplusplus
extern "C" {
#endif
#if(_WIN32_WINNT >= 0x0600)
#ifdef _WS2DEF_
extern CONST UCHAR sockaddr_size[AF_MAX];
MSWSOCKDEF_INLINE
UCHAR
SOCKADDR_SIZE(ADDRESS_FAMILY af)
{
    return (UCHAR)((af < AF_MAX) ? sockaddr_size[af]
                                 : sockaddr_size[AF_UNSPEC]);
}
MSWSOCKDEF_INLINE
SCOPE_LEVEL
ScopeLevel(
    IN SCOPE_ID ScopeId
    )
{
    //
    // We can't declare the Level field of type SCOPE_LEVEL directly,
    // since it gets sign extended to be negative.  We can, however,
    // safely cast.
    //
    return (SCOPE_LEVEL)ScopeId.Level;
}
#endif // _WS2DEF_
#define SIO_SET_COMPATIBILITY_MODE  _WSAIOW(IOC_VENDOR,300)
typedef enum _WSA_COMPATIBILITY_BEHAVIOR_ID {
    WsaBehaviorAll = 0,
    WsaBehaviorReceiveBuffering,
    WsaBehaviorAutoTuning
} WSA_COMPATIBILITY_BEHAVIOR_ID, *PWSA_COMPATIBILITY_BEHAVIOR_ID;

typedef struct _WSA_COMPATIBILITY_MODE {
    WSA_COMPATIBILITY_BEHAVIOR_ID BehaviorId;
    ULONG TargetOsVersion;
} WSA_COMPATIBILITY_MODE, *PWSA_COMPATIBILITY_MODE;   

#endif //(_WIN32_WINNT>=0x0600)
#ifdef __cplusplus
}
#endif
#ifdef MSWSOCKDEF_ASSERT_UNDEFINED
#undef ASSERT
#endif
#pragma option pop /*P_O_Pop*/
#endif  /* _MSWSOCKDEF_ */
#endif

#include <mswsock.h>
#endif

#if HAVE_WS2TCPIP_H
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
#include <ws2tcpip.h>
#ifdef __BORLANDC__
#pragma option pop
#endif
#endif

#ifdef __BORLANDC__
#pragma option pop
#pragma option pop
#pragma option pop
#endif

#if SIZEOF_ADDRINFO == 0
#ifndef __MINGW32__
typedef struct addrinfo
{
    int                ai_flags;
    int                ai_family;
    int                ai_socktype;
    int                ai_protocol;
    size_t             ai_addrlen;
    char *             ai_canonname;
    struct sockaddr *   ai_addr;
    struct addrinfo *   ai_next;
};
#endif

typedef struct addrinfoW
{
    int                ai_flags;
    int                ai_family;
    int                ai_socktype;
    int                ai_protocol;
    size_t             ai_addrlen;
    PWSTR              ai_canonname;
    struct sockaddr *   ai_addr;
    struct addrinfoW *   ai_next;
} ADDRINFOW, *PADDRINFOW;

#define SIZEOF_ADDRINFO sizeof(addrinfo)
#endif

#if SIZEOF_ADDRINFOA == 0
typedef struct addrinfo ADDRINFOA, *PADDRINFOA;
#define SIZEOF_ADDRINFOA sizeof(ADDRINFOA)
#endif

#if SIZEOF_ADDRINFOW == 0
typedef addrinfoW ADDRINFOW, *LPADDRINFOW;
#define SIZEOF_ADDRINFOW sizeof(ADDRINFOW)
#endif

#if HAVE_WINDOWS_H
#include <windows.h>
#endif

#if HAVE_TLHELP32_H
#include <tlhelp32.h>
#endif

#if HAVE_IPHLPAPI_H
#include <Iphlpapi.h>
#endif

#if HAVE_WBEMIDL_H
#include <WbemIdl.h>
#endif

#if HAVE_DBGHELP_H
#define _NO_CVCONST_H
#include <dbghelp.h>
#endif

#if HAVE_WINTERNL_H
#include <winternl.h>
#endif

#if HAVE_RAS_H
#include <ras.h>
#endif

/*#if defined(HAVE_WINDOWS_H) && !defined(__CYGWIN__)
#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0502
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0502
#endif
#ifndef NOT_INCLUDE_WINDOWS_H
#if __BCPLUSPLUS__
#pragma option push -w-8027
#endif
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <tlhelp32.h>
#if __BCPLUSPLUS__
#pragma option pop
#endif
#define NOT_INCLUDE_WINDOWS_H
#endif
#include <Iphlpapi.h>
#endif*/

#if HAVE_DIRENT_H
#include <dirent.h>
#endif

#ifdef __GNUG__
#define DECLSPEC_NOTHROW
#define GNUG_NOTHROW __attribute__((nothrow))
#ifdef __x86_64__
#define GNUG_CDECL
#define GNUG_NAKED
#else
#define GNUG_CDECL __attribute__((cdecl))
#define GNUG_NAKED __attribute__((naked))
#endif
#define GNUG_CONSTRUCTOR __attribute__((constructor))
#define GNUG_DESTRUCTOR __attribute__((destructor))
#ifndef DECLSPEC_NORETURN
#define DECLSPEC_NORETURN
#endif
#define GNUG_NORETURN __attribute__((noreturn))
#else
#ifndef DECLSPEC_NOTHROW
#define DECLSPEC_NOTHROW __declspec(nothrow)
#endif
#define GNUG_NOTHROW
#define GNUG_CDECL
#define GNUG_CONSTRUCTOR
#define GNUG_DESTRUCTOR
#ifndef DECLSPEC_NORETURN
#define DECLSPEC_NORETURN __declspec(noreturn)
#endif
#define GNUG_NORETURN
#define GNUG_NAKED
#endif
//---------------------------------------------------------------------------
#ifdef __cplusplus
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
typedef HANDLE file_t;
typedef SOCKET sock_t;
#else
typedef int sock_t;
typedef int file_t;
#define INVALID_HANDLE_VALUE -1
#define INVALID_SOCKET -1
#endif
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
template <typename T> inline void kxchg(T & v1, T & v2)
{
  T v(v1);
  v1 = v2;
  v2 = v;
}
//---------------------------------------------------------------------------
template <typename T> inline const T & kmin(const T & v1, const T & v2)
{
  return v1 < v2 ? v1 : v2;
}
//---------------------------------------------------------------------------
template <typename T> inline const T & kmax(const T & v1, const T & v2)
{
  return v1 > v2 ? v1 : v2;
}
//---------------------------------------------------------------------------
template <typename T> inline const T kabs(const T & v)
{
  return v > 0 ? v : -v;
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline
#endif
void reverseByteArray(void * array,uintptr_t size)
{
  for( uintptr_t i = size; i > size / 2; i-- )
    kxchg(((uint8_t *) array) [i - 1],((uint8_t *) array) [size - i]);
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline
#endif
void reverseByteArray(void * dst,const void * src,uintptr_t size)
{
  if( dst == src ){
    reverseByteArray(dst,size);
  }
  else {
    for( intptr_t i = size - 1; i >= 0; i-- )
      ((uint8_t *) dst) [size - i - 1] = ((const uint8_t *) src) [i];
  }
}
//---------------------------------------------------------------------------
typedef int32_t ilock_t;
ilock_t interlockedIncrement(volatile ilock_t & v,ilock_t a);
//------------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
extern "C" {
#endif
//---------------------------------------------------------------------------
#if !HAVE__MALLOC_OPTIONS
extern const char * _malloc_options;
#endif
//---------------------------------------------------------------------------
#if !HAVE_GETPAGESIZE
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__) || defined(__MINGW32__)
//---------------------------------------------------------------------------
static __inline uintptr_t getpagesize()
{
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  return si.dwPageSize;
}
//---------------------------------------------------------------------------
#elif HAVE_SYSCONF && defined(_SC_PAGESIZE)
//---------------------------------------------------------------------------
static __inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGESIZE);
}
//---------------------------------------------------------------------------
#elif HAVE_SYSCONF && defined(_SC_PAGE_SIZE)
//---------------------------------------------------------------------------
static __inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGE_SIZE);
}
//---------------------------------------------------------------------------
#elif __i386__
//---------------------------------------------------------------------------
static __inline uintptr_t getpagesize()
{
  return 4096;
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
static __inline uintptr_t getpagesize()
{
  return 8192;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#ifdef __cplusplus
};
#endif
//---------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------
