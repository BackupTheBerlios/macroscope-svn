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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//---------------------------------------------------------------------------
#if !HAVE__MALLOC_OPTIONS
const char * _malloc_options;
#endif
//---------------------------------------------------------------------------
#ifdef __cplusplus
};
#endif
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
void * kmalloc(size_t size,bool noThrow)
{
  void * p = NULL;
  if( size > 0 ){
//    p = malloc(size);
    p = MemoryManager::globalMemoryManager().malloc(size);
    if( p == NULL && !noThrow ){
      newObjectV1C2<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        __PRETTY_FUNCTION__ + utf8::String(" ") + utf8::int2Str(size)
      )->throwSP();
    }
  }
  return p;
}
//---------------------------------------------------------------------------
void * krealloc(void * p,size_t size,bool noThrow)
{
  if( p == NULL ) return kmalloc(size,noThrow);
  void * a;
  if( size == 0 ){
    kfree(p);
    a = NULL;
  }
  else {
//    a = realloc(p,size);
    a = MemoryManager::globalMemoryManager().realloc(p,size);
    if( a == NULL && !noThrow )
      newObjectV1C2<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(size)
      )->throwSP();
  }
  return a;
}
//---------------------------------------------------------------------------
void kfree(void * p)
{
  if( p != NULL ){
//    free(p);
    MemoryManager::globalMemoryManager().free(p);
  }
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
