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
#ifndef ksysH
#define ksysH
//---------------------------------------------------------------------------
//#define PRIVATE_RELEASE 1
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
template <typename T> inline void xchg(T & v1, T & v2)
{
  T v(v1);
  v1 = v2;
  v2 = v;
}
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#include <adicpp/lconfig.h>
#include <adicpp/bits.h>
#include <adicpp/autoptr.h>
#include <adicpp/xalloc.h>
#include <adicpp/xtime.h>
#include <adicpp/sp.h>
#include <adicpp/rnd.h>
#include <adicpp/tlv.h>
#include <adicpp/hash.h>
#include <adicpp/array.h>
#include <adicpp/list.h>
#include <adicpp/mutex.h>
#include <adicpp/thread.h>
#include <adicpp/profiler.h>
#include <adicpp/utf8str.h>
#include <adicpp/except.h>
#include <adicpp/tree.h>
#include <adicpp/qsort.h>
#include <adicpp/vector.h>
#include <adicpp/hol.h>
#include <adicpp/ehash.h>
#include <adicpp/table.h>
#include <adicpp/mstream.h>
#include <adicpp/mutant.h>
#include <adicpp/sysutils.h>
#include <adicpp/fhc.h>
#include <adicpp/sockaddr.h>
#include <adicpp/async.h>
#include <adicpp/afd.h>
#include <adicpp/log.h>
#include <adicpp/conf.h>
#include <adicpp/shsem.h>
#include <adicpp/fiber.h>
#include <adicpp/scktintf.h>
#include <adicpp/sha.h>
#include <adicpp/lzo.h>
#include <adicpp/socket.h>
#include <adicpp/shmem.h>
#include <adicpp/service.h>
#include <adicpp/fetch.h>
#include <adicpp/archive.h>
//---------------------------------------------------------------------------
#endif /* ksysH */
//---------------------------------------------------------------------------
