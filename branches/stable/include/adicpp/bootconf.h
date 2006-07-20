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

#if (defined(_MSC_VER) || defined(__INTEL_COMPILER)) && (defined(_WIN32) || defined(_WIN32_WINNT))
#include <adicpp/config.h.msvc>
#elif defined(__MINGW32__)
#include <adicpp/config.h.mingw>
#include <adicpp/config.h.cygwin>
#elif defined(__BORLANDC__)
#include <adicpp/config.h.borland>
#elif HAVE_CONFIG_H
#include <adicpp/config.h>
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

#endif /* _bootconf_H_ */
