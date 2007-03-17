/*-
 * Copyright 2005-2007 Guram Dukashvili
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
#include "lconfig.h"
#include "endian.h"
#include "object.h"
#include "xalloc.h"
#include "atomic.h"
#include "bits.h"
#include "autoptr.h"
#include "sp.h"
#include "utf8str.h"
#include "xtime.h"
#include "rnd.h"
#include "tlv.h"
#include "hash.h"
#include "array.h"
#include "list.h"
#include "mutex.h"
#include "thread.h"
#include "profiler.h"
#include "except.h"
//#include "tree.h"
#include "qsort.h"
#include "vector.h"
#include "hol.h"
#include "ehash.h"
#include "mstream.h"
#include "mutant.h"
#include "sysutils.h"
#include "table.h"
#include "sockaddr.h"
#include "shsem.h"
#include "async.h"
#include "afd.h"
#include "log.h"
#include "conf.h"
#include "fiber.h"
#include "scktintf.h"
#include "sha.h"
#include "lzo.h"
#include "socket.h"
#include "shmem.h"
#include "service.h"
#include "fetch.h"
#include "archive.h"
#include "radialer.h"
#include "varcont.h"
#if defined(__WIN32__) || defined(__WIN64__)
#include "pdbutils/pdbutils.h"
#endif
//---------------------------------------------------------------------------
#endif /* ksysH */
//---------------------------------------------------------------------------
