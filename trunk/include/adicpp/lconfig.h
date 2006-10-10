/*-
 * Copyright 2005 Guram Dukashvili
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

#ifndef _lconfig_H_
#define _lconfig_H_

//#define _ANSI_SOURCE
//#define _XOPEN_SOURCE 600
//#define _POSIX_C_SOURCE 200112

#if HAVE_STDARG_H
#include <stdarg.h>
#elif HAVE_VARARGS_H
#include <varargs.h>
#endif

#if HAVE_SYSEXITS_H
#include <sysexits.h>
#endif

#if HAVE_SYS_PARAM_H
#include <sys/param.h>
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

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#elif HAVE_SYS_TIME_H
#include <sys/time.h>
#elif HAVE_TIME_H
#include <time.h>
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

#if HAVE_UUID_H
#include <uuid.h>
#elif HAVE_SYS_UUID_H
#include <sys/uuid.h>
#endif

#if HAVE_SYS_EVENT_H
#include <sys/event.h>
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

#if HAVE_WINSOCK2_H
#include <winsock2.h>
#elif HAVE_WINSOCK_H
#include <winsock.h>
#endif

#if HAVE_MSWSOCK_H
#include <mswsock.h>
#endif

#if HAVE_WS2TCPIP_H
#include <ws2tcpip.h>
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

#endif /* _lconfig_H_ */
