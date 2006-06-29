dnl
dnl Process this file with autoconf to produce a configure script.
dnl

AC_PREREQ([2.59])
AC_INIT([libadicpp],[1.0.0],[korvin@tehnomir.lipetsk.ru])
AC_CONFIG_AUX_DIR(tools)

# Extract version
AC_PROG_SHTOOL

if test `id -g -nr` = korvin; then
  if test `hostname -s` = blackhole; then
    ./nv.sh l
  fi
fi

LIBADICPP_VERSION_STR=`$ac_cv_shtool version -l c -d long include/adicpp/version.h`
$ac_cv_shtool echo -e \
  "Configuring %Blibadicpp%b (Abstract Database Interface), version %B${LIBADICPP_VERSION_STR}%b"
#AC_SUBST(LIBADICPP_VERSION_STR)

AC_CANONICAL_TARGET
dnl AC_PREFIX_DEFAULT(/usr/local/lib/libadicpp)
AC_CONFIG_SRCDIR(src/libksys/xalloc.cpp)
AC_CONFIG_HEADERS(include/adicpp/config.h)
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(optimization,
  AC_HELP_STRING([--disable-optimization],[remove GNU Compiler optimization flags])
)

AC_ARG_WITH(firebird-include,
  AC_HELP_STRING([--with-firebird-include],[path to Firebird headers])
)

AC_ARG_WITH(firebird-static,
  AC_HELP_STRING([--with-firebird-static],[path to Firebird static library])
)

AC_ARG_ENABLE(firebird-static,
  AC_HELP_STRING([--enable-firebird-static],[enable Firebird static library link])
)
   
AC_ARG_WITH(mysql-include,
  AC_HELP_STRING([--with-mysql-include],[path to MySQL headers])
)

AC_ARG_WITH(mysql-static,
  AC_HELP_STRING([--with-mysql-static],[path to MySQL static library])
)

AC_ARG_ENABLE(mysql-static,
  AC_HELP_STRING([--enable-mysql-static], [enable MySQL static library link])
)

sinclude(codepage.m4)
      
# Checks for programs.
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
#AC_PROG_RANLIB

# Libtool init.
AC_ENABLE_STATIC
AC_DISABLE_SHARED
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# Select default language
AC_LANG_CPLUSPLUS

dnl #define _ANSI_SOURCE
dnl #define _XOPEN_SOURCE 600
dnl #define _POSIX_C_SOURCE 200112

ac_includes_default="\
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_UIO_H
#include <sys/uio.h>
#endif
#if HAVE_AIO_H
#include <aio.h>
#endif
#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#elif HAVE_CTIME
#include <ctime>
#else
#include <time.h>
#endif
#endif
#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif
#if HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#if HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif
#if HAVE_NETDB_H
#include <netdb.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_ERRNO_H
#include <errno.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
$ac_includes_default"

dnl if test ! -n "$CCC"; then
dnl    CCC="g++41 g++40 g++34 g++33 g++32"
dnl fi

# Checks for libraries.
isp="/usr/local/lib/mysql /usr/local/firebird/lib /usr/local/lib"
isp="$with_firebird_static $isp"
isp="$with_mysql_static $isp"
for i in $isp; do
  if test -d "$i"; then
    if test "$ac_cv_cxx_compiler_gnu" = yes; then
      LIBS="${LIBS} -L${i}";
    fi
  fi
done

AC_CHECK_LIB([m],[floor])
AC_CHECK_LIB([dl],[dlopen])
AC_CHECK_LIB([pthread],[pthread_exit])
dnl Some builds of MySQL require libz - try to detect
AC_CHECK_LIB([z],[deflate])
AC_CHECK_LIB([md],[SHA256_Init])
AM_CONDITIONAL([BUILD_SHA256_LIB],[test "$ac_cv_lib_SHA256_Init" = no])

AC_CHECK_LIB([lzo2],[lzo1x_999_compress_level])
if test "$ac_cv_lib_lzo2_lzo1x_999_compress_level" = no; then
  AC_CHECK_LIB([lzo],[lzo1x_999_compress_level])
fi
if test "$ac_cv_lib_lzo2_lzo1x_999_compress_level" = no; then
  if test "$ac_cv_lib_lzo_lzo1x_999_compress_level" = no; then
    am_cond=yes
  fi
fi
AM_CONDITIONAL([BUILD_LZO_LIB],[test "$am_cond" = yes])

if test "$enable_firebird_static" = yes; then
  AC_CHECK_LIB([fbstatic],[isc_attach_database])
  if test "$ac_cv_lib_fbstatic_isc_attach_database" = yes; then
    AC_DEFINE([FIREBIRD_STATIC_LIBRARY],[1],[Define to 1 if you want link Firebird static library.])
  fi
fi

if test "$enable_mysql_static" = yes; then
  AC_CHECK_LIB([mysqlclient_r],[mysql_thread_init])
  if test "$ac_cv_lib_mysqlclient_r_mysql_thread_init" = yes; then
    AC_DEFINE([MYSQL_STATIC_LIBRARY],[1],[Define to 1 if you want link MySQL static library.])
  fi
fi

# Checks for header files.
isp_fb="$with_firebird_include /opt/firebird/include /usr/local/firebird/include"
isp_my="$with_mysql_include /usr/include/mysql /usr/local/include/mysql"
isp_lzo="$with_lzo_include /opt/lzo/include /usr/local/include/lzo"
isp="$isp_fb $isp_my $isp_lzo /usr/local/include"
for i in $isp; do
  if test -d "$i"; then
    if test "$ac_cv_cxx_compiler_gnu" = yes; then
      CPPFLAGS="${CXXFLAGS} -I$i";
      CXXFLAGS="${CXXFLAGS} -I$i";
    fi
  fi
done

AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS(
ibase.h \
mysql.h \
lzo1x.h \
lzo_asm.h \
utime.h \
dirent.h \
sysexits.h \
sys/endian.h \
sys/types.h \
sys/ipc.h \
sys/sem.h \
sys/param.h \
sys/utsname.h \
sys/socket.h \
sys/event.h \
sys/mman.h \
sys/uuid.h \
sys/timeb.h \
aio.h \
sha256.h \
netinet/in.h \
arpa/inet.h \
netdb.h \
pwd.h \
grp.h \
limits.h \
io.h \
err.h \
errno.h \
float.h \
fcntl.h \
stdarg.h \
stdio.h \
math.h \
locale.h \
pthread.h \
signal.h \
windows.h \
typeinfo.h \
dlfcn.h \
assert.h \
semaphore.h \
process.h \
ctype.h \
stdint.h \
inttypes.h \
unistd.h \
malloc.h \
stddef.h \
paths.h \
stdexept \
new \
typeinfo
)

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_UID_T
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_C_VOLATILE
AC_C_LONG_DOUBLE
AC_C_BIGENDIAN
AC_TYPE_SIGNAL
AC_C_STRINGIZE
AC_CHECK_TYPES([ptrdiff_t,struct sockaddr_in6,struct aiocb])
AC_CHECK_MEMBERS([struct stat.st_rdev,struct sockaddr_in.sin_len,struct sockaddr_in6.sin6_len])

#AC_PATH_X
#AC_PATH_XTRA
#AC_SYS_LARGEFILE
#AC_SYS_LONG_FILE_NAMES
#AC_SYS_POSIX_TERMIOS

AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([bool])
AC_CHECK_SIZEOF([char],[1])
AC_CHECK_SIZEOF([wchar_t])
AC_CHECK_SIZEOF([short])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])
AC_CHECK_SIZEOF([long int])
AC_CHECK_SIZEOF([long double])
AC_CHECK_SIZEOF([intptr_t])
AC_CHECK_SIZEOF([intmax_t])
AC_CHECK_SIZEOF([ptrdiff_t])
AC_CHECK_SIZEOF([socklen_t])
AC_CHECK_SIZEOF([struct timeval])
AC_CHECK_SIZEOF([struct timezone])
AC_CHECK_SIZEOF([uid_t])
AC_CHECK_SIZEOF([gid_t])

# Checks for library functions.
AC_FUNC_CHOWN
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRERROR_R
AC_FUNC_UTIME_NULL
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(\
localtime_s \
gmtime_s \
getpwnam \
getgrnam \
floor \
floorl \
ftruncate \
getcwd \
getpagesize \
gettimeofday \
memmove \
memset \
memcpy \
strcasecmp \
strchr \
strncasecmp \
strstr \
sprintf \
_sprintf \
vsprintf \
snprintf \
_snprintf \
vsnprintf \
sscanf \
_sscanf \
snscanf \
_snscanf \
chsize \
fstat \
nanosleep \
sleep \
usleep \
pread \
pwrite \
getuid \
getgid \
getpid \
gettid \
sem_init \
sem_close \
sem_open \
sem_unlink \
uname \
umask \
readdir_r \
socket \
mkdir \
inet_ntoa \
gethostbyname \
gethostbyname2 \
gethostbyaddr \
getnameinfo \
getaddrinfo \
strerror \
utime \
sysconf \
sigwait \
rmdir \
isatty \
fileno \
kqueue \
kevent \
aio_read \
aio_write \
aio_return \
aio_error \
mmap \
munmap \
shm_open \
shm_unlink \
daemon \
uuidgen \
semget \
semop \
semctl \
ftok \
exit \
pthread_create \
pthread_detach \
pthread_self \
pthread_yield \
pthread_join \
pthread_setcancelstate \
pthread_attr_setguardsize \
pthread_attr_getstack \
pthread_getprio \
pthread_setprio \
pthread_getschedparam \
pthread_setschedparam \
pthread_rwlock_init \
pthread_rwlock_destroy \
pthread_rwlock_rdlock \
pthread_rwlock_tryrdlock \
pthread_rwlock_wrlock \
pthread_rwlock_trywrlock \
pthread_rwlock_unlock \
lzo1x_999_compress_level \
lzo1x_decompress \
lzo1x_decompress_safe \
lzo1x_decompress_asm \
lzo1x_decompress_asm_safe \
lzo1x_decompress_asm_fast \
lzo1x_decompress_asm_fast_safe)

# My checks
AC_MSG_CHECKING([intptr_t type equal int type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct intptr_t_check { void f(intptr_t); void f(int); }],
  iasi=no,
  AC_DEFINE([HAVE_INTPTR_T_AS_INT],[1],[Define to 1 if you have intptr_t type equal int type.])
)
AC_MSG_RESULT([$iasi])

if test "$ac_cv_type_long_int" = yes; then
  AC_MSG_CHECKING([intptr_t type equal long int type])
  iasi=yes
  AC_TRY_COMPILE(
    [$ac_includes_default],
    [struct intptr_t_check { void f(intptr_t); void f(long int); }],
    iasi=no,
    AC_DEFINE([HAVE_INTPTR_T_AS_LONG_INT],[1],[Define to 1 if you have intptr_t type equal long int type.])
  )
  AC_MSG_RESULT([$iasi])
fi

if test "$ac_cv_type_long_long" = yes; then
  AC_MSG_CHECKING([intptr_t type equal long long type])
  iasi=yes
  AC_TRY_COMPILE(
    [$ac_includes_default],
    [struct intptr_t_check { void f(intptr_t); void f(long long); }],
    iasi=no,
    AC_DEFINE([HAVE_INTPTR_T_AS_LONG_INT],[1],[Define to 1 if you have intptr_t type equal long long type.])
  )
  AC_MSG_RESULT([$iasi])
fi

AC_MSG_CHECKING([intptr_t type equal intmax_t type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct intptr_t_check { void f(intptr_t); void f(intmax_t); }],
  iasi=no,
  AC_DEFINE([HAVE_INTPTR_T_AS_INTMAX_T],[1],[Define to 1 if you have intptr_t type equal intmax_t type.])
)
AC_MSG_RESULT([$iasi])

AC_MSG_CHECKING([intptr_t type equal int32_t type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct intptr_t_check { void f(intptr_t); void f(int32_t); }],
  iasi=no,
  AC_DEFINE([HAVE_INTPTR_T_AS_INT32_T],[1],[Define to 1 if you have intptr_t type equal int32_t type.])
)
AC_MSG_RESULT([$iasi])

AC_MSG_CHECKING([intptr_t type equal int64_t type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct intptr_t_check { void f(intptr_t); void f(int64_t); }],
  iasi=no,
  AC_DEFINE([HAVE_INTPTR_T_AS_INT64_T],[1],[Define to 1 if you have intptr_t type equal int64_t type.])
)
AC_MSG_RESULT([$iasi])

AC_MSG_CHECKING([int32_t type equal int type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct int32_t_check { void f(int32_t); void f(int); }],
  iasi=no,
  AC_DEFINE([HAVE_INT32_T_AS_INT],[1],[Define to 1 if you have int32_t type equal int type.])
)
AC_MSG_RESULT([$iasi])

AC_MSG_CHECKING([int64_t type equal intmax_t type])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct int64_t_check { void f(int64_t); void f(intmax_t); }],
  iasi=no,
  AC_DEFINE([HAVE_INT64_T_AS_INTMAX_T],[1],[Define to 1 if you have int64_t type equal intmax_t type.])
)
AC_MSG_RESULT([$iasi])

AC_MSG_CHECKING([C++ compiler empty array support])
iasi=yes
AC_TRY_COMPILE(
  [$ac_includes_default],
  [struct { int foo; int bar[0]; } mystruct],
  AC_DEFINE_UNQUOTED(EMPTY_ARRAY_SIZE,0,[Dimension to use for empty array declaration.]),
  AC_TRY_COMPILE(
    [],
    [struct { int foo; int bar@<:@@:>@; } mystruct],
    AC_DEFINE_UNQUOTED(EMPTY_ARRAY_SIZE,,[Dimension to use for empty array declaration.]),
    iasi=no
  )
)
AC_MSG_RESULT([$iasi])

if test "$ac_cv_func_sem_open" = yes; then
  if test "$ac_cv_header_signal_h" = yes; then
    AC_MSG_CHECKING([sem_open call return ENOSYS])
    iasi=yes
    AC_RUN_IFELSE(
      AC_LANG_PROGRAM(
        [
          #include <stdio.h>
          #include <stdlib.h>
          #if HAVE_SEMAPHORE_H
          #include <semaphore.h>
          #endif
          #if HAVE_SIGNAL_H
          #include <signal.h>
          #endif
          #if HAVE_ERRNO_H
          #include <errno.h>
          #endif
          #if HAVE_SYS_STAT_H
          #include <sys/stat.h>
          #endif
          #if HAVE_FCNTL_H
          #include <fcntl.h>
          #endif
	],
        [
  	  sigset_t ss;
	  sigemptyset(&ss);
	  sigaddset(&ss,SIGSYS);
	  sigprocmask(SIG_BLOCK,&ss,NULL);
          errno = 0;
          sem_t * sem = sem_open("/ac_tc_sem",O_CREAT,S_IWUSR);
          int err = errno;
          sem_unlink("/ac_tc_sem");
          if( err != ENOSYS ) return ENOSYS;
        ]
      ),
      AC_DEFINE_UNQUOTED(HAVE_SEM_OPEN_ENOSYS,1,[Define to 1 if you have sem_open call return ENOSYS.]),
      iasi=no,
      iasi=no
    )
    AC_MSG_RESULT([$iasi])
  fi
fi

if test "$enable_optimization" = "no"; then
  if test "$ac_cv_cxx_compiler_gnu" = yes; then
    flags=
    for i in $CFLAGS; do
      case "$i" in
        -g*) ;;
        -O*) ;;
          *) flags="${flags} ${i}";;
      esac
    done
    flags="${flags} -O0 -fbounds-check -fstack-check -fno-function-cse -fshort-enums -D_REENTRANT -D_THREAD_SAFE -Wall"
    if test "$ac_cxx_werror_flag" = yes; then
	flags="${flags} -Werror"
    fi
    if test "$ac_cv_prog_cc_g" = yes; then
      case "$target_os" in
        *freebsd*)
          flags="${flags} -gstabs+"
	;;
        *)
          flags="${flags} -g"
        ;;
      esac
    fi
    CFLAGS="${flags}"
    flags=
    for i in $CXXFLAGS; do
      case "$i" in
        -g*) ;;
        -O*) ;;
          *) flags="${flags} ${i}";;
      esac
    done
    flags="${flags} -O0 -fbounds-check -fstack-check -fno-function-cse -fshort-enums -D_REENTRANT -D_THREAD_SAFE -Wall"
    if test "$ac_cxx_werror_flag" = yes; then
	flags="${flags} -Werror"
    fi
    if test "$ac_cv_prog_cxx_g" = yes; then
      case "$target_os" in
        *freebsd*)
          flags="${flags} -gstabs+"
	;;
        *)
          flags="${flags} -g"
        ;;
      esac
    fi
    CXXFLAGS="${flags}"
  fi
else
  if test "$ac_cv_cxx_compiler_gnu" = yes; then
dnl -ftree-vectorize -fdump-tree-vect-stats -march=prescott -msse3 -m3dnow -finline-limit=99999
dnl -finline-functions, -fweb and -frename-registers
    flags=
    for i in $CFLAGS; do
      case "$i" in
        -g*) ;;
        -O*) ;;
          *) flags="${flags} ${i}";;
      esac
    done
    flags="${flags} -fshort-enums -DNDEBUG -D_REENTRANT -D_THREAD_SAFE -Wall"
    if test "$ac_cxx_werror_flag" = yes; then
	flags="${flags} -Werror"
    fi
    case "$target_os" in
      *freebsd*)
        flags="${flags} -O2 -fomit-frame-pointer"
      ;;
      *)
        flags="${flags} -O3"
      ;;
    esac
    CFLAGS="${flags}"
    flags=
    for i in $CXXFLAGS; do
      case "$i" in
        -g*) ;;
        -O*) ;;
          *) flags="${flags} ${i}";;
      esac
    done
    flags="${flags} -fshort-enums -DNDEBUG -D_REENTRANT -D_THREAD_SAFE -Wall"
    if test "$ac_cxx_werror_flag" = yes; then
	flags="${flags} -Werror"
    fi
    case "$target_os" in
      *freebsd*)
        flags="${flags} -O2 -finline-functions -fomit-frame-pointer"
      ;;
      *)
        flags="${flags} -O3"
      ;;
    esac
    CXXFLAGS="${flags}"
  fi
fi

case "$target_os" in
  *solaris*)
    if test "$ac_cv_cxx_compiler_gnu" = yes; then
      CXXFLAGS="${CXXFLAGS} -D_POSIX_PTHREAD_SEMANTICS"
    fi
  ;;
  *freebsd*)
    LIBS="${LIBS} -pthread"
  ;;
esac

CPPFLAGS=
CXXFLAGS="${CXXFLAGS} -DSYSCONF_DIR=${sysconfdir}/"

# Create output
AC_CONFIG_FILES([
Makefile
builds/Makefile
builds/windows/Makefile
builds/windows/BCB6/Makefile
builds/windows/VC80/Makefile
include/Makefile
include/adicpp/Makefile
include/adicpp/fbapi/Makefile
include/adicpp/lzo/Makefile
include/adicpp/myapi/Makefile
include/adicpp/sha/Makefile
bugs/Makefile
examples/Makefile
doc/Makefile
src/Makefile
src/libksys/Makefile
src/libutf8/Makefile
src/libfbcpp/Makefile
src/libmycpp/Makefile
src/libadicpp/Makefile
test/Makefile
test/dlopen/Makefile
test/fiber/Makefile
test/genutf8/Makefile
test/kqueue/Makefile
test/macroscope/Makefile
test/macroscope/BCB6/Makefile
test/macroscope/VC60/Makefile
test/macroscope/VC70/Makefile
test/msftp/Makefile
test/msftp/BCB6/Makefile
test/msftp/VC70/Makefile
test/msftpd/Makefile
test/msftpd/BCB6/Makefile
test/msftpd/VC70/Makefile
test/uss/Makefile
])

AC_OUTPUT
