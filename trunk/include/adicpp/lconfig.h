/*-
 * Copyright 2005-2008 Guram Dukashvili
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
#include "bootconf.h"
//---------------------------------------------------------------------------
#ifndef _lconfig_H_
#define _lconfig_H_
//---------------------------------------------------------------------------
#if HAVE_SYSLOG_H
#include <syslog.h>
#endif

#if HAVE_EXECINFO_H
#include <execinfo.h>
#else
#include <adicpp/execinfo/execinfo.h>
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
#define DECLSPEC_NORETURN
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
namespace ksys {
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
template <typename T> inline void xchg(T & v1, T & v2)
{
  T v(v1);
  v1 = v2;
  v2 = v;
}
//---------------------------------------------------------------------------
template <typename T> inline const T & tmin(const T & v1, const T & v2)
{
  return v1 < v2 ? v1 : v2;
}
//---------------------------------------------------------------------------
template <typename T> inline const T & tmax(const T & v1, const T & v2)
{
  return v1 > v2 ? v1 : v2;
}
//---------------------------------------------------------------------------
template <typename T> inline T tabs(const T & v)
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
    xchg(((uint8_t *) array) [i - 1],((uint8_t *) array) [size - i]);
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
inline
#endif
void reverseByteArray(void * dst,const void * src,uintptr_t size)
{
  if( dst == src ){
    for( uintptr_t i = size; i > size / 2; i-- )
  	  xchg(((uint8_t *) dst) [i - 1],((uint8_t *) dst) [size - i]);
  }
  else {
    for( intptr_t i = size - 1; i >= 0; i-- )
      ((uint8_t *) dst) [size - i - 1] = ((const uint8_t *) src) [i];
  }
}
//---------------------------------------------------------------------------
} // namespace ksys
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
#if defined(__WIN32__) || defined(__WIN64__)
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
static inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGESIZE);
}
//---------------------------------------------------------------------------
#elif HAVE_SYSCONF && defined(_SC_PAGE_SIZE)
//---------------------------------------------------------------------------
static inline uintptr_t getpagesize()
{
  return sysconf(_SC_PAGE_SIZE);
}
//---------------------------------------------------------------------------
#elif __i386__
//---------------------------------------------------------------------------
static inline uintptr_t getpagesize()
{
  return 4096;
}
//---------------------------------------------------------------------------
#else
//---------------------------------------------------------------------------
static inline uintptr_t getpagesize()
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
#endif /* _lconfig_H_ */
//---------------------------------------------------------------------------
