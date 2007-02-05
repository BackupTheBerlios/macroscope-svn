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
#ifndef hashH
#define hashH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
template <class T> class HashFuncContainer {
  public:
    static uintptr_t  hash(const void * key, uintptr_t len, uintptr_t h = 0);
    static uintptr_t  hash_rev(const void * key, uintptr_t len, uintptr_t h = 0);
    static uint64_t   hash_ll(const void * key, uintptr_t len, uint64_t h = 0);
    static uint64_t   hash_rev_ll(const void * key, uintptr_t len, uint64_t h = 0);
    static uintptr_t  hash(const char * s, uintptr_t h = 0);
    static uint64_t   hash_ll(const char * s, uint64_t h = 0);
    //    static uintptr_t hash(const void * key,uintptr_t h);
    static uintptr_t  hash(intptr_t key, uintptr_t h = 0);
    static uintptr_t  hash(uintptr_t key, uintptr_t h = 0);
};
typedef HashFuncContainer<int> HF;
//---------------------------------------------------------------------------
//#define HASH4(h,k,d) (h) = ((h) << 5) + (h) + *(const unsigned char *) (k); *reinterpret_cast<const void **>(&(k)) = (const unsigned char *) (k) + (d);
//#define HASH5(h,k,d) (h) = ((h) << 5) + (h) + *((const unsigned char *) (k) + (d));
template <typename T1,typename T2,typename T3> inline
void HASH4(T1 & h, T2 & k, T3 d)
{
  h = (h << 5) + h + *(const uint8_t *) k;
  *reinterpret_cast< const void **>(&k) = (const uint8_t *) k + d;
}
template <typename T1,typename T2,typename T3> inline
void HASH5(T1 & h, T2 k, T3 d)
{
  h = (h << 5) + h + *((const uint8_t *) k + d);
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t HashFuncContainer<T>::hash(const void * key, uintptr_t len, uintptr_t h)
{
  const void *  pk  = key;
  key = (const uint8_t *) key + len;
  switch( len & 7 ){
    case 0:
      while( (const uint8_t *) pk < (const uint8_t *) key ){
        HASH4(h, pk, 1);
        case 7:
        HASH4(h, pk, 1);
        case 6:
        HASH4(h, pk, 1);
        case 5:
        HASH4(h, pk, 1);
        case 4:
        HASH4(h, pk, 1);
        case 3:
        HASH4(h, pk, 1);
        case 2:
        HASH4(h, pk, 1);
        case 1:
        HASH4(h, pk, 1);
      }
  }
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
 inline
#endif
uintptr_t HashFuncContainer<T>::hash_rev(const void * key, uintptr_t len, uintptr_t h)
{
  const void *  pk  = (const uint8_t *) key + len - 1;
  switch( len & 7 ){
    case 0:
      while( (const uint8_t *) pk > (const uint8_t *) key ){
        HASH4(h, pk, -1);
        case 7:
        HASH4(h, pk, -1);
        case 6:
        HASH4(h, pk, -1);
        case 5:
        HASH4(h, pk, -1);
        case 4:
        HASH4(h, pk, -1);
        case 3:
        HASH4(h, pk, -1);
        case 2:
        HASH4(h, pk, -1);
        case 1:
        HASH4(h, pk, -1);
      }
  }
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t HashFuncContainer<T>::hash(const char * s,uintptr_t h)
{
  for( ; ; ){
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
  }
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uint64_t HashFuncContainer<T>::hash_ll(const char * s,uint64_t h)
{
  for(;;){
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
    if( *s == '\0' ) break;
    HASH4(h,s,1);
  }
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uint64_t HashFuncContainer<T>::hash_ll(const void * key, uintptr_t len, uint64_t h)
{
  const void *  pk  = key;
  key = (const uint8_t *) key + len;
  switch( len & 7 ){
    case 0:
      while( (const uint8_t *) pk < (const uint8_t *) key ){
        HASH4(h, pk, 1);
        case 7:
        HASH4(h, pk, 1);
        case 6:
        HASH4(h, pk, 1);
        case 5:
        HASH4(h, pk, 1);
        case 4:
        HASH4(h, pk, 1);
        case 3:
        HASH4(h, pk, 1);
        case 2:
        HASH4(h, pk, 1);
        case 1:
        HASH4(h, pk, 1);
      }
  }
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
uint64_t HashFuncContainer<T>::hash_rev_ll(const void * key, uintptr_t len, uint64_t h)
{
  const void *  pk  = (const uint8_t *) key + len - 1;
  switch( len & 7 ){
    case 0:
      while( (const uint8_t *) pk > (const uint8_t *) key ){
        HASH4(h, pk, -1);
        case 7:
        HASH4(h, pk, -1);
        case 6:
        HASH4(h, pk, -1);
        case 5:
        HASH4(h, pk, -1);
        case 4:
        HASH4(h, pk, -1);
        case 3:
        HASH4(h, pk, -1);
        case 2:
        HASH4(h, pk, -1);
        case 1:
        HASH4(h, pk, -1);
      }
  }
  return h;
}
//---------------------------------------------------------------------------
/*template <class T> inline
uintptr_t HashFuncContainer<T>::hash(const void * key,uintptr_t h)
{
  HASH5(h,&key,0);
  HASH5(h,&key,1);
  HASH5(h,&key,2);
  HASH5(h,&key,3);
#if SIZEOF_INTPTR_T > 4
  HASH5(h,&key,4);
  HASH5(h,&key,5);
  HASH5(h,&key,6);
  HASH5(h,&key,7);
#endif
  return h;
}*/
//---------------------------------------------------------------------------
template <class T> inline
uintptr_t HashFuncContainer<T>::hash(intptr_t key, uintptr_t h)
{
  HASH5(h, &key, 0);
  HASH5(h, &key, 1);
  HASH5(h, &key, 2);
  HASH5(h, &key, 3);
#if SIZEOF_INTPTR_T > 4
  HASH5(h, &key, 4);
  HASH5(h, &key, 5);
  HASH5(h, &key, 6);
  HASH5(h, &key, 7);
#endif
  return h;
}
//---------------------------------------------------------------------------
template <class T> inline
uintptr_t HashFuncContainer<T>::hash(uintptr_t key, uintptr_t h)
{
  HASH5(h, &key, 0);
  HASH5(h, &key, 1);
  HASH5(h, &key, 2);
  HASH5(h, &key, 3);
#if SIZEOF_INTPTR_T > 4
  HASH5(h, &key, 4);
  HASH5(h, &key, 5);
  HASH5(h, &key, 6);
  HASH5(h, &key, 7);
#endif
  return h;
}
//---------------------------------------------------------------------------
template <class T>
#ifndef __BCPLUSPLUS__
inline
#endif
T hashT(const void * key, uintptr_t len, T h = 0)
{
  const void *  pk  = (const uint8_t *) key + len - 1;
  switch( len & 7 ){
    case 0:
      while( (const uint8_t *) pk > (const uint8_t *) key ){
        HASH4(h, pk, -1);
        case 7:
        HASH4(h, pk, -1);
        case 6:
        HASH4(h, pk, -1);
        case 5:
        HASH4(h, pk, -1);
        case 4:
        HASH4(h, pk, -1);
        case 3:
        HASH4(h, pk, -1);
        case 2:
        HASH4(h, pk, -1);
        case 1:
        HASH4(h, pk, -1);
      }
  }
  return h;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
