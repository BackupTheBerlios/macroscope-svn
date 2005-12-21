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
#define GUARD_SIZE 0
//---------------------------------------------------------------------------
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
static void setNewMemoryBlock(void * & p,size_t size)
{
  uint8_t * & p8 = *(uint8_t **) &p;
  *(void **) p = p;
  *(size_t *) (p8 + sizeof(void *)) = size;
  *(size_t *) (p8 + GUARD_SIZE / 2 - sizeof(size_t)) = size - GUARD_SIZE;
  memset(
    p8 + sizeof(void *) + sizeof(size_t),
    0,
    GUARD_SIZE / 2 - sizeof(void *) - sizeof(size_t) * 2
  );
  memset(
    p8 + size - GUARD_SIZE / 2,
    0,
    GUARD_SIZE / 2
  );
  p = p8 + GUARD_SIZE / 2;
}
#endif
//---------------------------------------------------------------------------
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
static void checkMemoryBlock(void * p,size_t & size1,size_t & size2,void * & a)
{
  a = NULL;
  if( p != NULL ){
    uint8_t * & p8 = *(uint8_t **) &p;
    size1 = *(size_t *) (p8 - sizeof(size_t));
    size2 = *(size_t *) (p8 - GUARD_SIZE / 2 + sizeof(void *));
    assert( size1 + GUARD_SIZE == size2 );
    a = *(void **) (p8 - GUARD_SIZE / 2);
    assert( (uint8_t *) a == p8 - GUARD_SIZE / 2 );
    intptr_t i;
    for( i = GUARD_SIZE / 2 - 1; i >= 0; i-- ){
      assert( p8[size1 + i] == 0 );
    }
    for( i = GUARD_SIZE / 2 - sizeof(void *) - sizeof(size_t) * 2 - 1; i >= 0; i-- ){
      assert( p8[sizeof(void *) + sizeof(size_t) - GUARD_SIZE / 2 + i] == 0 );
    }
  }
}
#endif
//---------------------------------------------------------------------------
void * kmalloc(size_t size)
{
  void * p = NULL;
  if( size > 0 ){
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
    size += GUARD_SIZE;
#endif
#if __BCPLUSPLUS__
    giant().acquire();
#endif
    p = malloc(size);
#if __BCPLUSPLUS__
    giant().release();
#endif
    if( p == NULL ){
      throw ksys::ExceptionSP(
        new EOutOfMemory(
          ENOMEM,
          utf8::string(__PRETTY_FUNCTION__) + " " + utf8::int2Str((intmax_t) size)
        )
      );
    }
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
    else {
//      memset(p,0,size);
      setNewMemoryBlock(p,size);
    }
#endif
  }
  return p;
}
//---------------------------------------------------------------------------
void * krealloc(void * p,size_t size)
{
  void * a;
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
  size_t size1, size2;
  checkMemoryBlock(p,size1,size2,a);
  if( a != NULL ) p = a;
  if( size > 0 ){
    size += GUARD_SIZE;
  }
  else {
    if( a != NULL ) memset(a,0,size2);
  }
#endif
  if( size == 0 ){
    if( p != NULL ){
#if __BCPLUSPLUS__
      giant().acquire();
#endif
      free(p);
#if __BCPLUSPLUS__
      giant().release();
#endif
    }
    a = NULL;
  }
  else {
    if( p == NULL ){
#if __BCPLUSPLUS__
      giant().acquire();
#endif
      a = malloc(size);
#if __BCPLUSPLUS__
      giant().release();
#endif
    }
    else {
#if __BCPLUSPLUS__
      giant().acquire();
#endif
      a = realloc(p,size);
#if __BCPLUSPLUS__
      giant().release();
#endif
    }
    if( a == NULL )
      throw ksys::ExceptionSP(
        new EOutOfMemory(
          ENOMEM,utf8::string(__PRETTY_FUNCTION__) + " " + utf8::int2Str((intmax_t) size)
        )
      );
  }
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
  if( a != NULL ) setNewMemoryBlock(a,size);
#endif
  return a;
}
//---------------------------------------------------------------------------
void kfree(void * p)
{
#if ((__BCPLUSPLUS && defined(_DEBUG)) || !defined(NDEBUG)) && GUARD_SIZE > 0
  void * a;
  size_t size1, size2;
  checkMemoryBlock(p,size1,size2,a);
  if( a != NULL ){
    memset(a,0,size2);
    p = a;
  }
#endif
#if __BCPLUSPLUS__
  giant().acquire();
#endif
  free(p);
#if __BCPLUSPLUS__
  giant().release();
#endif
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// heap manager /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
struct HeapBlockDesc {
  void ** block_;
  uintptr_t freeChunk_;
};
//---------------------------------------------------------------------------
struct HeapBlockBySizeDesc {
  uintptr_t count_;
  HeapBlockDesc * blocks_;
  uintptr_t freeBlock_;
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
