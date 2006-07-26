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
#ifndef _bootconf_H_
#define _bootconf_H_

#if HAVE_CONFIG_H
#include <adicpp/config.h>
#elif (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && (defined(_WIN32) || defined(_WIN32_WINNT))
#include <adicpp/config.h.msvc>
#elif defined(__MINGW32__)
#include <adicpp/config.h.mingw>
#include <adicpp/config.h.cygwin>
#elif defined(__BORLANDC__)
#include <adicpp/config.h.borland>
#else
#error Not configured
#endif

#ifdef __CYGWIN__
#ifdef __WIN32__
#undef __WIN32__
#endif
#ifdef __WIN64__
#undef __WIN64__
#endif
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

#ifndef SIZEOF_WCHAR_T
#define SIZEOF_WCHAR_T 0
#endif

#if SIZEOF_WCHAR_T == 0
typedef short wchar_t;
#undef SIZEOF_WCHAR_T
#define SIZEOF_WCHAR_T sizeof(short)
#endif

#if !defined(PACKED) && defined(__GNUG__)
#define PACKED __attribute__ ((packed))
#endif

#ifndef SIZEOF_CHAR
#define SIZEOF_CHAR 0
#endif

#if SIZEOF_CHAR != 1
#error size of char != 1
#endif

#ifndef SIZEOF_SHORT
#define SIZEOF_SHORT 0
#endif

#ifndef SIZEOF_INT
#define SIZEOF_INT 0
#endif

#ifndef SIZEOF_LONG
#define SIZEOF_LONG 0
#endif

#ifndef SIZEOF_LONG_LONG
#define SIZEOF_LONG_LONG 0
#endif

#ifndef SIZEOF_LONG_INT
#define SIZEOF_LONG_INT 0
#endif

#ifndef SIZEOF_LONG_DOUBLE
#define SIZEOF_LONG_DOUBLE 0
#endif

#ifndef SIZEOF_INTPTR_T
#define SIZEOF_INTPTR_T 0
#endif

#ifndef SIZEOF_PTRDIFF_T
#define SIZEOF_PTRDIFF_T 0
#endif

#ifndef SIZEOF_INT8_T
#define SIZEOF_INT8_T 0
#endif

#ifndef SIZEOF__INT8
#define SIZEOF__INT8 0
#endif

#ifndef SIZEOF_INT16_T
#define SIZEOF_INT16_T 0
#endif

#ifndef SIZEOF__INT16
#define SIZEOF__INT16 0
#endif

#ifndef SIZEOF_INT32_T
#define SIZEOF_INT32_T 0
#endif

#ifndef SIZEOF__INT32
#define SIZEOF__INT32 0
#endif

#ifndef SIZEOF_INT64_T
#define SIZEOF_INT64_T 0
#endif

#ifndef SIZEOF__INT64
#define SIZEOF__INT64 0
#endif

#ifndef SIZEOF_INTMAX_T
#define SIZEOF_INTMAX_T 0
#endif

#ifndef SIZEOF_LONG_DOUBLE
#define SIZEOF_LONG_DOUBLE 0
#endif

#if SIZEOF_INT8_T == 0
#if SIZEOF__INT8 == 1
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
#elif SIZEOF_CHAR == 1
typedef char int8_t;
typedef unsigned char uint8_t;
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
#elif SIZEOF_INT == 4
typedef int int32_t;
typedef unsigned int uint32_t;
#define HAVE_INT32_T_AS_INT 1
#else
#error sizeof(int) != 4
#endif
#undef SIZEOF_INT32_T
#define SIZEOF_INT32_T 4
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
#endif

#if SIZEOF_INTMAX_T == 0
typedef int64_t intmax_t;
typedef uint64_t uintmax_t;
#undef SIZEOF_INTMAX_T
#define SIZEOF_INTMAX_T 8
#define HAVE_INTMAX_T_AS_INT64_T 1
#define HAVE_INT64_T_AS_INTMAX_T 1
#endif

#if SIZEOF_INTPTR_T == 0
#undef SIZEOF_INTPTR_T
#if SIZEOF_INT32_T == SIZEOF_VOID_P
typedef int32_t intptr_t;
typedef unsigned int32_t uintptr_t;
#define SIZEOF_INTPTR_T 4
#define HAVE_INTPTR_T_AS_INT32_T 1
#elif SIZEOF_INT64_t == SIZEOF_VOID_P
typedef int64_t intptr_t;
typedef unsigned int64_t uintptr_t;
#define SIZEOF_INTPTR_T 8
#define HAVE_INTPTR_T_AS_INT64_T 1
#define HAVE_INTPTR_T_AS_INTMAX_T 1
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

#if SIZEOF_LONG_DOUBLE > 0
#define HAVE_LONG_DOUBLE 1
#endif

#ifndef NDEBUG
#undef SYSCONF_DIR
#endif

#ifndef SYSCONF_DIR
#define SYSCONF_DIR ksys::getExecutablePath()
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

#ifndef _WINDOWS
#define _WINDOWS 1
#endif

#if defined(__WIN32__) || defined(__WIN64__)
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

#if defined(__WIN32__) && !defined(__WIN64__)
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

#if _MSC_VER < 1300 && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ ""
#elif _MSC_VER >= 1300 && !defined(__PRETTY_FUNCTION__)
#define __PRETTY_FUNCTION__ __FUNCSIG__
#endif

#ifndef PACKED
#define PACKED
#endif

#endif /* _bootconf_H_ */
