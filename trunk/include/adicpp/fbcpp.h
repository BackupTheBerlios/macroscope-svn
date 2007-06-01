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
#ifndef fbcppH
#define fbcppH

#include <adicpp/ksys.h>

#if HAVE_IBASE_H
#include <ibase.h>
#else
#include <adicpp/fbapi/ibase.h>
#endif

#if FB_API_VER < 20
typedef char ISC_SCHAR;
#endif

#define SQL_FB_FLOAT 482
#undef SQL_FLOAT
#define SQL_FB_DOUBLE 480
#undef SQL_DOUBLE
#define SQL_FB_TYPE_DATE 570
#undef SQL_TYPE_DATE
#define SQL_FB_TYPE_TIME 560
#undef SQL_TYPE_TIME
#define SQL_FB_DATE 510
#undef SQL_DATE
#define SQL_FB_TIMESTAMP 510
#undef SQL_TIMESTAMP

#include <adicpp/fbintf.h>
#include <adicpp/fbdb.h>
#include <adicpp/fbexcpt.h>
#include <adicpp/fbtr.h>
#include <adicpp/fbst.h>
#include <adicpp/fbsvc.h>
#include <adicpp/fbevent.h>
#include <adicpp/fbexcept.h>

namespace fbcpp {

extern void initialize();
extern void cleanup();

}

#endif /* fbcppH */
