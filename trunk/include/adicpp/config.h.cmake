#cmakedefine HAVE__FSEEKI64 1
#cmakedefine HAVE_FLOAT_H 1
#cmakedefine HAVE_STDIO_H 1
#cmakedefine HAVE_STDLIB_H 1
#cmakedefine HAVE_MALLOC_H 1
#cmakedefine HAVE_MEMORY_H 1
#cmakedefine HAVE_SYS_MOUNT_H 1
#cmakedefine HAVE_DIRENT_H 1
#cmakedefine HAVE_ERROR_H 1
#cmakedefine HAVE_IBASE_H 1
#cmakedefine HAVE_PCAP_H 1
#cmakedefine HAVE_MYSQL_H 1
#cmakedefine HAVE_MYSQL_MYSQL_H 1
#cmakedefine HAVE_SQL_H 1
#cmakedefine HAVE_GD_H 1
#cmakedefine HAVE_LZO_LZO1X_H 1
#cmakedefine HAVE_LZO1X_H 1
#cmakedefine HAVE_LZO_LZO_ASM_H 1
#cmakedefine HAVE_LZO_ASM_H 1
#cmakedefine HAVE_UTIME_H 1
#cmakedefine HAVE_SYS_UTIME_H 1
#cmakedefine HAVE_SYSEXITS_H 1
#cmakedefine HAVE_SYS_ENDIAN_H 1
#cmakedefine HAVE_SYS_TYPES_H 1
#cmakedefine HAVE_SYS_IPC_H 1
#cmakedefine HAVE_SYS_SEM_H 1
#cmakedefine HAVE_SYS_PARAM_H 1
#cmakedefine HAVE_SYS_UTSNAME_H 1
#cmakedefine HAVE_SYS_SOCKET_H 1
#cmakedefine HAVE_SYS_EVENT_H 1
#cmakedefine HAVE_SYS_MMAN_H 1
#cmakedefine HAVE_SYS_UUID_H 1
#cmakedefine HAVE_SYS_TIME_H 1
#cmakedefine HAVE_SYS_RESOURCE_H 1
#cmakedefine HAVE_SYS_TIMEB_H 1
#cmakedefine HAVE_SYS_STAT_H 1
#cmakedefine HAVE_SYS_IOCTL_H 1
#cmakedefine HAVE_SYS_SYSCTL_H 1
#cmakedefine HAVE_SYS_EPOLL_H 1
#cmakedefine HAVE_SYS_RTPRIO_H 1
#cmakedefine HAVE_MACHINE_ATOMIC_H 1
#cmakedefine HAVE_MACHINE_CPUFUNC_H 1
#cmakedefine HAVE_MACHINE_SPECIALREG_H 1
#cmakedefine HAVE_UCONTEXT_H 1
#cmakedefine HAVE_SYSLOG_H 1
#cmakedefine HAVE_EXECINFO_H 1
#cmakedefine HAVE_TIME_H 1
#cmakedefine HAVE_SCHED_H 1
#cmakedefine HAVE_UUID_H 1
#cmakedefine HAVE_AIO_H 1
#cmakedefine HAVE_SHA256_H 1
#cmakedefine HAVE_NETINET_IN_H 1
#cmakedefine HAVE_NETINET_IN_SYSTM_H 1
#cmakedefine HAVE_NETINET_IP_H 1
#cmakedefine HAVE_NETINET_TCP_H 1
#cmakedefine HAVE_NET_IF_H 1
#cmakedefine HAVE_NET_IF_TYPES_H 1
#cmakedefine HAVE_NET_ROUTE_H 1
#cmakedefine HAVE_NET_ETHERNET_H 1
#cmakedefine HAVE_NET_IF_DL_H 1
#cmakedefine HAVE_ARPA_INET_H 1
#cmakedefine HAVE_NETDB_H 1
#cmakedefine HAVE_PWD_H 1
#cmakedefine HAVE_GRP_H 1
#cmakedefine HAVE_LIMITS_H 1
#cmakedefine HAVE_IO_H 1
#cmakedefine HAVE_ERR_H 1
#cmakedefine HAVE_ERRNO_H 1
#cmakedefine HAVE_FLOAT_H 1
#cmakedefine HAVE_FCNTL_H 1
#cmakedefine HAVE_STDARG_H 1
#cmakedefine HAVE_MATH_H 1
#cmakedefine HAVE_LOCALE_H 1
#cmakedefine HAVE_PTHREAD_H 1
#cmakedefine HAVE_SIGNAL_H 1
#cmakedefine HAVE_WS2TCPIP_H 1
#cmakedefine HAVE_WINSOCK_H 1
#cmakedefine HAVE_WINSOCK2_H 1
#cmakedefine HAVE_WINDOWS_H 1
#cmakedefine HAVE_NTDLL_H 1
#cmakedefine HAVE_NTSTATUS_H 1
#cmakedefine HAVE_MSWSOCK_H 1
#cmakedefine HAVE_WBEMIDL_H 1
#cmakedefine HAVE_DBGHELP_H 1
#cmakedefine HAVE_NTSTATUS_H 1
#cmakedefine HAVE_WINTERNL_H 1
#cmakedefine HAVE_TLHELP32_H 1
#cmakedefine HAVE_IPHLPAPI_H 1
#cmakedefine HAVE_TYPEINFO_H 1
#cmakedefine HAVE_DLFCN_H 1
#cmakedefine HAVE_ASSERT_H 1
#cmakedefine HAVE_SEMAPHORE_H 1
#cmakedefine HAVE_PROCESS_H 1
#cmakedefine HAVE_CTYPE_H 1
#cmakedefine HAVE_STDINT_H 1
#cmakedefine HAVE_INTTYPES_H 1
#cmakedefine HAVE_UNISTD_H 1
#cmakedefine HAVE_STDDEF_H 1
#cmakedefine HAVE_PATHS_H 1
#cmakedefine HAVE_STDEXCEPT 1
#cmakedefine HAVE_NEW 1
#cmakedefine HAVE_TYPEINFO 1
#cmakedefine HAVE_DEMANGLE_H 1
#cmakedefine HAVE_LIBIBERTY_DEMANGLE_H 1

#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1
#endif

#ifndef BIG_ENDIAN
#define BIG_ENDIAN 2
#endif

#ifndef BYTE_ORDER
#cmakedefine BYTE_ORDER ${BYTE_ORDER}
#endif

#cmakedefine SIZEOF_PTRDIFF_T ${SIZEOF_PTRDIFF_T}
#cmakedefine SIZEOF_AIOCB ${SIZEOF_AIOCB}
#cmakedefine SIZEOF_UUID ${SIZEOF_UUID}
#cmakedefine SIZEOF_GUID ${SIZEOF_GUID}
#cmakedefine SIZEOF_VOID_P ${SIZEOF_VOID_P}
#cmakedefine SIZEOF_BOOL ${SIZEOF_BOOL}
#cmakedefine SIZEOF_ULONG ${SIZEOF_ULONG}
#cmakedefine SIZEOF_CHAR ${SIZEOF_CHAR}
#cmakedefine SIZEOF_WCHAR_T ${SIZEOF_WCHAR_T}
#cmakedefine SIZEOF_SHORT ${SIZEOF_SHORT}
#cmakedefine SIZEOF_INT ${SIZEOF_INT}
#cmakedefine SIZEOF_LONG ${SIZEOF_LONG}
#cmakedefine SIZEOF_LONG_LONG ${SIZEOF_LONG_LONG}
#cmakedefine SIZEOF_LONG_INT ${SIZEOF_LONG_INT}
#cmakedefine SIZEOF_LONG_DOUBLE ${SIZEOF_LONG_DOUBLE}
#cmakedefine SIZEOF_INTPTR_T ${SIZEOF_INTPTR_T}
#cmakedefine SIZEOF_INTMAX_T ${SIZEOF_INTMAX_T}
#cmakedefine SIZEOF_SOCKLEN_T ${SIZEOF_SOCKLEN_T}
#cmakedefine SIZEOF_PID_T ${SIZEOF_PID_T}
#cmakedefine SIZEOF_UID_T ${SIZEOF_UID_T}
#cmakedefine SIZEOF_GID_T ${SIZEOF_GID_T}
#cmakedefine SIZEOF_UUID_T ${SIZEOF_UUID_T}
#cmakedefine SIZEOF__INT8 ${SIZEOF__INT8}
#cmakedefine SIZEOF__INT16 ${SIZEOF__INT16}
#cmakedefine SIZEOF__INT32 ${SIZEOF__INT32}
#cmakedefine SIZEOF__INT64 ${SIZEOF__INT64}
#cmakedefine SIZEOF_INT8_T ${SIZEOF_INT8_T}
#cmakedefine SIZEOF_INT16_T ${SIZEOF_INT16_T}
#cmakedefine SIZEOF_INT32_T ${SIZEOF_INT32_T}
#cmakedefine SIZEOF_INT64_T ${SIZEOF_INT64_T}
#cmakedefine SIZEOF_INTMAX_T ${SIZEOF_INTMAX_T}
#cmakedefine SIZEOF_TIMEVAL ${SIZEOF_TIMEVAL}
#cmakedefine SIZEOF_TIMEZONE ${SIZEOF_TIMEZONE}
#cmakedefine SIZEOF_SOCKADDR_IN6 ${SIZEOF_SOCKADDR_IN6}
#cmakedefine SIZEOF_SOCKADDR_DL ${SIZEOF_SOCKADDR_DL}
#cmakedefine SIZEOF_STRUCT_AIOCB ${SIZEOF_STRUCT_AIOCB}
#cmakedefine SIZEOF_STAT64 ${SIZEOF_STAT64}
#cmakedefine SIZEOF__STAT64 ${SIZEOF__STAT64}
#cmakedefine SIZEOF_KEVENT ${SIZEOF_KEVENT}
#cmakedefine SIZEOF_UTIMBUF ${SIZEOF_UTIMBUF}
#cmakedefine SIZEOF_UCONTEXT_T ${SIZEOF_UCONTEXT_T}

#cmakedefine HAVE_CHOWN 1
#cmakedefine HAVE_OPENDIR 1
#cmakedefine HAVE_CLOSEDIR 1
#cmakedefine HAVE_MALLOC 1
#cmakedefine HAVE_REALLOC 1
#cmakedefine HAVE_MEMCMP 1
#cmakedefine HAVE_MKTIME 1
#cmakedefine HAVE_TIMEGM 1
#cmakedefine HAVE_STAT 1
#cmakedefine HAVE_STRERROR_R 1
#cmakedefine HAVE_UTIME 1
#cmakedefine HAVE__UTIME 1
#cmakedefine HAVE_UTIMES 1
#cmakedefine HAVE_VPRINTF 1
#cmakedefine HAVE_LOCALTIME 1
#cmakedefine HAVE_GMTIME_S 1
#cmakedefine HAVE_GMTIME 1
#cmakedefine HAVE_GETPWNAM 1
#cmakedefine HAVE_GETGRNAM 1
#cmakedefine HAVE_FLOOR 1
#cmakedefine HAVE_FLOORL 1
#cmakedefine HAVE_FTRUNCATE 1
#cmakedefine HAVE_GETCWD 1
#cmakedefine HAVE_GETPAGESIZE 1
#cmakedefine HAVE_GETTIMEOFDAY 1
#cmakedefine HAVE_MEMMOVE 1
#cmakedefine HAVE_MEMSET 1
#cmakedefine HAVE_MEMCPY 1
#cmakedefine HAVE_BCOPY 1
#cmakedefine HAVE_STRCASECMP 1
#cmakedefine HAVE_STRCHR 1
#cmakedefine HAVE_STRNCASECMP 1
#cmakedefine HAVE_STRSTR 1
#cmakedefine HAVE_SPRINTF 1
#cmakedefine HAVE__SPRINTF 1
#cmakedefine HAVE_VSPRINTF 1
#cmakedefine HAVE_SNPRINTF 1
#cmakedefine HAVE__SNPRINTF 1
#cmakedefine HAVE_VSNPRINTF 1
#cmakedefine HAVE_SSCANF 1
#cmakedefine HAVE__SSCANF 1
#cmakedefine HAVE_SNSCANF 1
#cmakedefine HAVE__SNSCANF 1
#cmakedefine HAVE__CHSIZE 1
#cmakedefine HAVE_CHSIZE 1
#cmakedefine HAVE_FSTAT 1
#cmakedefine HAVE_NANOSLEEP 1
#cmakedefine HAVE_SLEEP 1
#cmakedefine HAVE_USLEEP 1
#cmakedefine HAVE_PREAD 1
#cmakedefine HAVE_PWRITE 1
#cmakedefine HAVE_GETUID 1
#cmakedefine HAVE_GETGID 1
#cmakedefine HAVE_GETPID 1
#cmakedefine HAVE_GETTID 1
#cmakedefine HAVE_SEM_INIT 1
#cmakedefine HAVE_SEM_CLOSE 1
#cmakedefine HAVE_SEM_OPEN 1
#cmakedefine HAVE_SEM_UNLINK 1
#cmakedefine HAVE_SEM_TIMEDWAIT 1
#cmakedefine HAVE_UNAME 1
#cmakedefine HAVE_UMASK 1
#cmakedefine HAVE_READDIR_R 1
#cmakedefine HAVE_SOCKET 1
#cmakedefine HAVE_MKDIR 1
#cmakedefine HAVE_INET_NTOA 1
#cmakedefine HAVE_GETHOSTBYNAME 1
#cmakedefine HAVE_GETHOSTBYNAME2 1
#cmakedefine HAVE_GETHOSTBYADDR 1
#cmakedefine HAVE_GETNAMEINFO 1
#cmakedefine HAVE_GETADDRINFO 1
#cmakedefine HAVE_STRERROR 1
#cmakedefine HAVE_SYSCONF 1
#cmakedefine HAVE_SIGWAIT 1
#cmakedefine HAVE_RMDIR 1
#cmakedefine HAVE__ISATTY 1
#cmakedefine HAVE_ISATTY 1
#cmakedefine HAVE__FILENO 1
#cmakedefine HAVE_FILENO 1
#cmakedefine HAVE_KQUEUE 1
#cmakedefine HAVE_KEVENT 1
#cmakedefine HAVE__MALLOC_OPTIONS 1
#cmakedefine HAVE_AIO_READ 1
#cmakedefine HAVE_AIO_WRITE 1
#cmakedefine HAVE_AIO_RETURN 1
#cmakedefine HAVE_AIO_ERROR 1
#cmakedefine HAVE_MMAP 1
#cmakedefine HAVE_MUNMAP 1
#cmakedefine HAVE_MLOCK 1
#cmakedefine HAVE_MUNLOCK 1
#cmakedefine HAVE_MLOCKALL 1
#cmakedefine HAVE_MUNLOCKALL 1
#cmakedefine HAVE_SHM_OPEN 1
#cmakedefine HAVE_SHM_UNLINK 1
#cmakedefine HAVE_DAEMON 1
#cmakedefine HAVE_UUIDGEN 1
#cmakedefine HAVE_UUID_CREATE 1
#cmakedefine HAVE_SEMGET 1
#cmakedefine HAVE_SEMOP 1
#cmakedefine HAVE_SEMCTL 1
#cmakedefine HAVE_FTOK 1
#cmakedefine HAVE_EXIT 1
#cmakedefine HAVE__TZSET 1
#cmakedefine HAVE_TZSET 1
#cmakedefine HAVE_BE16ENC 1
#cmakedefine HAVE_BE32ENC 1
#cmakedefine HAVE_BE64ENC 1
#cmakedefine HAVE_BE16TOH 1
#cmakedefine HAVE_BE32TOH 1
#cmakedefine HAVE_BE64TOH 1
#cmakedefine HAVE_LE16ENC 1
#cmakedefine HAVE_LE32ENC 1
#cmakedefine HAVE_LE64ENC 1
#cmakedefine HAVE_LE16TOH 1
#cmakedefine HAVE_LE32TOH 1
#cmakedefine HAVE_LE64TOH 1
#cmakedefine HAVE_BE16DEC 1
#cmakedefine HAVE_BE32DEC 1
#cmakedefine HAVE_BE64DEC 1
#cmakedefine HAVE_HTOBE16 1
#cmakedefine HAVE_HTOBE32 1
#cmakedefine HAVE_HTOBE64 1
#cmakedefine HAVE_LE16DEC 1
#cmakedefine HAVE_LE32DEC 1
#cmakedefine HAVE_LE64DEC 1
#cmakedefine HAVE_HTOLE16 1
#cmakedefine HAVE_HTOLE32 1
#cmakedefine HAVE_HTOLE64 1
#cmakedefine HAVE_UUID_FROM_STRING 1
#cmakedefine HAVE_CLOCK_GETRES 1
#cmakedefine HAVE_CLOCK_GETTIME 1
#cmakedefine HAVE_SETPRIORITY 1
#cmakedefine HAVE_GETPRIORITY 1
#cmakedefine HAVE_NICE 1
#cmakedefine HAVE_PTHREAD_CREATE 1
#cmakedefine HAVE_PTHREAD_DETACH 1
#cmakedefine HAVE_PTHREAD_SELF 1
#cmakedefine HAVE_PTHREAD_YIELD 1
#cmakedefine HAVE_PTHREAD_JOIN 1
#cmakedefine HAVE_PTHREAD_SETCANCELSTATE 1
#cmakedefine HAVE_PTHREAD_ATTR_SETGUARDSIZE 1
#cmakedefine HAVE_PTHREAD_ATTR_GETSTACK 1
#cmakedefine HAVE_PTHREAD_GETPRIO 1
#cmakedefine HAVE_PTHREAD_SETPRIO 1
#cmakedefine HAVE_PTHREAD_ATTR_GETSHEDPARAM 1
#cmakedefine HAVE_PTHREAD_ATTR_SETSHEDPARAM 1
#cmakedefine HAVE_PTHREAD_GETSCHEDPARAM 1
#cmakedefine HAVE_PTHREAD_SETSCHEDPARAM 1
#cmakedefine HAVE_PTHREAD_RWLOCK_INIT 1
#cmakedefine HAVE_PTHREAD_RWLOCK_DESTROY 1
#cmakedefine HAVE_PTHREAD_RWLOCK_RDLOCK 1
#cmakedefine HAVE_PTHREAD_TRYRDLOCK 1
#cmakedefine HAVE_PTHREAD_WRLOCK 1
#cmakedefine HAVE_PTHREAD_TRYWRLOCK 1
#cmakedefine HAVE_PTHREAD_UNLOCK 1
#cmakedefine HAVE_SHA256_INIT 1
#cmakedefine HAVE_LZO1X_999_COMPRESS_LEVEL 1
#cmakedefine HAVE_LZO1X_DECOMPRESS 1
#cmakedefine HAVE_LZO1X_DECOMPRESS_SAFE 1
#cmakedefine HAVE_LZO1X_DECOMPRESS_ASM 1
#cmakedefine HAVE_LZO1X_DECOMPRESS_ASM_SAFE 1
#cmakedefine HAVE_LZO1X_DECOMPRESS_ASM_FAST 1
#cmakedefine HAVE_LZO1X_DECOMPRESS_ASM_FAST_SAFE 1
#cmakedefine HAVE_DO_CPUID 1
#cmakedefine HAVE_ATOMIC_FETCHADD_32 1
#cmakedefine HAVE_RTPRIO 1

#cmakedefine EMPTY_ARRAY_SIZE ${EMPTY_ARRAY_SIZE}

#cmakedefine HAVE_INTPTR_T_AS_INT ${HAVE_INTPTR_T_AS_INT}
#cmakedefine HAVE_INTPTR_T_AS_LONG ${HAVE_INTPTR_T_AS_LONG}
#cmakedefine HAVE_INTPTR_T_AS_LONG_INT ${HAVE_INTPTR_T_AS_LONG_INT}
#cmakedefine HAVE_INTPTR_T_AS_LONG_LONG ${HAVE_INTPTR_T_AS_LONG_LONG}
#cmakedefine HAVE_INTPTR_T_AS_INT32_T ${HAVE_INTPTR_T_AS_INT32_T}
#cmakedefine HAVE_INTPTR_T_AS_INT64_T ${HAVE_INTPTR_T_AS_INT64_T}
#cmakedefine HAVE_INTPTR_T_AS_INTMAX_T ${HAVE_INTPTR_T_AS_INTMAX_T}

#cmakedefine HAVE_INT_AS_INTPTR_T ${HAVE_INT_AS_INTPTR_T}
#cmakedefine HAVE_INT_AS_LONG ${HAVE_INT_AS_LONG}
#cmakedefine HAVE_INT_AS_LONG_INT ${HAVE_INT_AS_LONG_INT}
#cmakedefine HAVE_INT_AS_LONG_LONG ${HAVE_INT_AS_LONG_LONG}
#cmakedefine HAVE_INT_AS_INT32_T ${HAVE_INT_AS_INT32_T}
#cmakedefine HAVE_INT_AS_INT64_T ${HAVE_INT_AS_INT64_T}
#cmakedefine HAVE_INT_AS_INTMAX_T ${HAVE_INT_AS_INTMAX_T}

#cmakedefine HAVE_LONG_AS_INTPTR_T ${HAVE_LONG_AS_INTPTR_T}
#cmakedefine HAVE_LONG_AS_LONG_INT ${HAVE_LONG_AS_LONG_INT}
#cmakedefine HAVE_LONG_AS_LONG_LONG ${HAVE_LONG_AS_LONG_LONG}
#cmakedefine HAVE_LONG_AS_INT32_T ${HAVE_LONG_AS_INT32_T}
#cmakedefine HAVE_LONG_AS_INT64_T ${HAVE_LONG_AS_INT64_T}
#cmakedefine HAVE_LONG_AS_INTMAX_T ${HAVE_LONG_AS_INTMAX_T}

#cmakedefine HAVE_LONG_INT_AS_INTPTR_T ${HAVE_LONG_INT_AS_INTPTR_T}
#cmakedefine HAVE_LONG_INT_AS_LONG ${HAVE_LONG_INT_AS_LONG}
#cmakedefine HAVE_LONG_INT_AS_LONG_LONG ${HAVE_LONG_INT_AS_LONG_LONG}
#cmakedefine HAVE_LONG_INT_AS_INT32_T ${HAVE_LONG_INT_AS_INT32_T}
#cmakedefine HAVE_LONG_INT_AS_INT64_T ${HAVE_LONG_INT_AS_INT64_T}
#cmakedefine HAVE_LONG_INT_AS_INTMAX_T ${HAVE_LONG_INT_AS_INTMAX_T}

#cmakedefine HAVE_INT32_T_AS_INTPTR_T ${HAVE_INT32_T_AS_INTPTR_T}
#cmakedefine HAVE_INT32_T_AS_LONG ${HAVE_INT32_T_AS_LONG}
#cmakedefine HAVE_INT32_T_AS_LONG_INT ${HAVE_INT32_T_AS_LONG_INT}
#cmakedefine HAVE_INT32_T_AS_LONG_LONG ${HAVE_INT32_T_AS_LONG_LONG}
#cmakedefine HAVE_INT32_T_AS_INTMAX_T ${HAVE_INT32_T_AS_INTMAX_T}

#cmakedefine HAVE_INT64_T_AS_INTPTR_T ${HAVE_INT64_T_AS_INTPTR_T}
#cmakedefine HAVE_INT64_T_AS_LONG ${HAVE_INT64_T_AS_LONG}
#cmakedefine HAVE_INT64_T_AS_LONG_INT ${HAVE_INT64_T_AS_LONG_INT}
#cmakedefine HAVE_INT64_T_AS_LONG_LONG ${HAVE_INT64_T_AS_LONG_LONG}
#cmakedefine HAVE_INT64_T_AS_INTMAX_T ${HAVE_INT64_T_AS_INTMAX_T}

#cmakedefine HAVE_INTMAX_T_AS_INTPTR_T ${HAVE_INTMAX_T_AS_INTPTR_T}
#cmakedefine HAVE_INTMAX_T_AS_INT ${HAVE_INTMAX_T_AS_INT}
#cmakedefine HAVE_INTMAX_T_AS_LONG ${HAVE_INTMAX_T_AS_LONG}
#cmakedefine HAVE_INTMAX_T_AS_LONG_INT ${HAVE_INTMAX_T_AS_LONG_INT}
#cmakedefine HAVE_INTMAX_T_AS_LONG_LONG ${HAVE_INTMAX_T_AS_LONG_LONG}
#cmakedefine HAVE_INTMAX_T_AS_INT64_T ${HAVE_INTMAX_T_AS_INT64_T}

#cmakedefine HAVE_SIGVAL_SIGVAL_PTR 1
#cmakedefine HAVE_SIGVAL_SIVAL_PTR 1

#cmakedefine PRIVATE_RELEASE 1
#cmakedefine CMAKE_BUILD 1
#cmakedefine CMAKE_GENERATOR_IS_VS 1
#cmakedefine CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_ENUM}
#cmakedefine CMAKE_BUILD_TYPE_LOWER "${CMAKE_BUILD_TYPE_LOWER}"
#cmakedefine CMAKE_BUILD_TYPE_UPPER "${CMAKE_BUILD_TYPE_UPPER}"
#cmakedefine CMAKE_SYSTEM_NAME "${CMAKE_SYSTEM_NAME_NORMALIZED}"
#cmakedefine CMAKE_SYSTEM_VERSION "${CMAKE_SYSTEM_VERSION_NORMALIZED}"
