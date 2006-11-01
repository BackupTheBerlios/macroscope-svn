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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#define NODEBUGMEM 1
//---------------------------------------------------------------------------
void * kmalloc(size_t size)
{
  void * p = NULL;
  if( size > 0 ){
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    p = malloc(size + sizeof(uintptr_t));
#else
    p = malloc(size);
#endif
    if( p == NULL ){
      newObject<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(size)
      )->throwSP();
    }
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    *(uintptr_t *) p = size;
    memset((uintptr_t *) p + 1,0xFF,size);
    p = (uintptr_t *) p + 1;
#endif
  }
  return p;
}
//---------------------------------------------------------------------------
void * krealloc(void * p, size_t size)
{
  if( p == NULL ) return kmalloc(size);
  void * a;
  if( size == 0 ){
    kfree(p);
    a = NULL;
  }
  else {
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    a = realloc((uintptr_t *) p - 1,size + sizeof(uintptr_t));
#else
    a = realloc(p,size);
#endif
    if( a == NULL )
      newObject<EOutOfMemory>(
#if defined(__WIN32__) || defined(__WIN64__)
        ERROR_NOT_ENOUGH_MEMORY + errorOffset,
#else
        ENOMEM,
#endif
        utf8::String(__PRETTY_FUNCTION__) + " " + utf8::int2Str(size)
      )->throwSP();
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    if( size > *(uintptr_t *) a )
      memset((uint8_t *) ((uintptr_t *) a + 1) + *(uintptr_t *) a,0xFF,size - *(uintptr_t *) a);
    *(uintptr_t *) a = size;
    a = (uintptr_t *) a + 1;
#endif
  }
  return a;
}
//---------------------------------------------------------------------------
void kfree(void * p)
{
  if( p != NULL ){
#if !defined(NDEBUG) && !defined(NODEBUGMEM)
    memset((uintptr_t *) p - 1,0xFF,*((uintptr_t *) p - 1) + sizeof(uintptr_t));
    p = (uintptr_t *) p - 1;
#endif
    free(p);
  }
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// heap manager /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct HeapBlockDesc {
    void **   block_;
    uintptr_t freeChunk_;
};
//---------------------------------------------------------------------------
struct HeapBlockBySizeDesc {
    uintptr_t       count_;
    HeapBlockDesc * blocks_;
    uintptr_t       freeBlock_;
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
