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
#ifndef _bits_H
#define _bits_H
//-----------------------------------------------------------------------------
namespace ksys {
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
inline void setBit(void * ptr,uintptr_t n)
{
  ((uint8_t *) ptr)[n >> 3] |= uint8_t(1u << (n & 7));
}
//-----------------------------------------------------------------------------
inline void setBit(void * ptr,uintmax_t n)
{
  ((uint8_t *) ptr)[uintptr_t(n) >> 3] |= uint8_t(1u << (uintptr_t(n) & 7));
}
//-----------------------------------------------------------------------------
inline void resetBit(void * ptr,uintptr_t n)
{
  ((uint8_t *) ptr)[n >> 3] &= uint8_t(~0x80u >> (~n & 7));
}
//-----------------------------------------------------------------------------
inline void resetBit(void * ptr,uintmax_t n)
{
  ((uint8_t *) ptr)[uintptr_t(n) >> 3] &= uint8_t(~0x80u >> (~uintptr_t(n) & 7));
}
//-----------------------------------------------------------------------------
inline void invertBit(void * ptr,uintptr_t n)
{
  ((uint8_t *) ptr)[n >> 3] |= uint8_t(~((uint8_t *) ptr)[n >> 3] & ~(1u << (n & 7)));
}
//-----------------------------------------------------------------------------
inline void invertBit(void * ptr,uintmax_t n)
{
  ((uint8_t *) ptr)[uintptr_t(n) >> 3] |= uint8_t(~((uint8_t *) ptr)[uintptr_t(n) >> 3] & ~(1u << (uintptr_t(n) & 7)));
}
//-----------------------------------------------------------------------------
inline uintptr_t bit(const void * ptr,uintptr_t n)
{
  return ((uint8_t *) ptr)[n >> 3] >> (n & 7) & 1u;
}
//-----------------------------------------------------------------------------
inline uintptr_t bit(const void * ptr,uintmax_t n)
{
  return ((uint8_t *) ptr)[uintptr_t(n) >> 3] >> (uintptr_t(n) & 7) & 1u;
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
void setBitRangeT(T * ptr,uintptr_t n,uintptr_t c)
{
  uintptr_t a = -intptr_t(n) & 7;
  uint8_t mask = ~0xFFu >> a;
  if( c < a ) mask &= 0xFFu >> (a - c);
  ((uint8_t *) ptr)[n >> 3] |= mask;
  c -= a;
  if( intptr_t(c) > 0 ){
    n += a;
    while( c >= SIZEOF_INTMAX_T * 8 ){
      *(uintmax_t *) ((uint8_t *) ptr + (n >> 3)) = ~uintmax_t(0u);
      n += SIZEOF_INTMAX_T * 8;
      c -= SIZEOF_INTMAX_T * 8;
    }
#if SIZEOF_INTMAX_T > 4
    if( c >= 4 * 8 ){
      *(uint32_t *) ((uint8_t *) ptr + (n >> 3)) = uint32_t(~uint32_t(0u));
      n += 4 * 8;
      c -= 4 * 8;
    }
#endif
    if( c >= 2 * 8 ){
      *(uint16_t *) ((uint8_t *) ptr + (n >> 3)) = uint16_t(~uint16_t(0u));
      n += 2 * 8;
      c -= 2 * 8;
    }
    if( c >= 8 ){
      *((uint8_t *) ptr + (n >> 3)) = uint8_t(~uint8_t(0u));
      n += 8;
      c -= 8;
    }
    if( c > 0 )
      ((uint8_t *) ptr)[n >> 3] |= uint8_t(0xFFu >> (-intptr_t(c) & 7));
  }
}
//-----------------------------------------------------------------------------
inline void setBitRange(void * ptr,uintptr_t n,uintptr_t c)
{
  setBitRangeT<void>(ptr,n,c);
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
void resetBitRangeT(T * ptr,uintptr_t n,uintptr_t c)
{
  uintptr_t a = -intptr_t(n) & 7;
  uint8_t mask = ~0xFFu >> a;
  if( c < a ) mask &= 0xFFu >> (a - c);
  ((uint8_t *) ptr)[n >> 3] &= uint8_t(~mask);
  c -= a;
  if( intptr_t(c) > 0 ){
    n += a;
    while( c >= SIZEOF_INTMAX_T * 8 ){
      *(uintmax_t *) ((uint8_t *) ptr + (n >> 3)) = 0;
      n += SIZEOF_INTMAX_T * 8;
      c -= SIZEOF_INTMAX_T * 8;
    }
#if SIZEOF_INTMAX_T > 4
    if( c >= 4 * 8 ){
      *(uint32_t *) ((uint8_t *) ptr + (n >> 3)) = 0;
      n += 4 * 8;
      c -= 4 * 8;
    }
#endif
    if( c >= 2 * 8 ){
      *(uint16_t *) ((uint8_t *) ptr + (n >> 3)) = 0;
      n += 2 * 8;
      c -= 2 * 8;
    }
    if( c >= 8 ){
      *((uint8_t *) ptr + (n >> 3)) = 0;
      n += 8;
      c -= 8;
    }
    if( c > 0 )
      ((uint8_t *) ptr)[n >> 3] &= uint8_t(~(0xFFu >> (-intptr_t(c) & 7)));
  }
}
//-----------------------------------------------------------------------------
inline void resetBitRange(void * ptr,uintptr_t n,uintptr_t c)
{
  resetBitRangeT<void>(ptr,n,c);
}
//-----------------------------------------------------------------------------
template <typename T>
#ifndef __BCPLUSPLUS__
inline
#endif
void invertBitRangeT(T * ptr,uintptr_t n,uintptr_t c)
{
  uintptr_t a = -intptr_t(n) & 7;
  uint8_t mask = ~0xFFu >> a;
  if( c < a ) mask &= 0xFFu >> (a - c);
  ((uint8_t *) ptr)[n >> 3] |= uint8_t(~((uint8_t *) ptr)[n >> 3] & mask);
  c -= a;
  if( intptr_t(c) > 0 ){
    n += a;
    while( c >= SIZEOF_INTMAX_T * 8 ){
      *(uintmax_t *) ((uint8_t *) ptr + (n >> 3)) = 
        ~*(uintmax_t *) ((uint8_t *) ptr + (n >> 3));
      n += SIZEOF_INTMAX_T * 8;
      c -= SIZEOF_INTMAX_T * 8;
    }
#if SIZEOF_INTMAX_T > 4
    if( c >= 4 * 8 ){
      *(uint32_t *) ((uint8_t *) ptr + (n >> 3)) =
        uint32_t(~*(uint32_t *) ((uint8_t *) ptr + (n >> 3)));
      n += 4 * 8;
      c -= 4 * 8;
    }
#endif
    if( c >= 2 * 8 ){
      *(uint16_t *) ((uint8_t *) ptr + (n >> 3)) =
        uint16_t(~*(uint16_t *) ((uint8_t *) ptr + (n >> 3)));
      n += 2 * 8;
      c -= 2 * 8;
    }
    if( c >= 8 ){
      *((uint8_t *) ptr + (n >> 3)) =
        uint8_t(~*(uint8_t *) ((uint8_t *) ptr + (n >> 3)));
      n += 8;
      c -= 8;
    }
    if( c > 0 )
      ((uint8_t *) ptr)[n >> 3] |=
        uint8_t(~((uint8_t *) ptr)[n >> 3] & ~(0xFFu >> (-intptr_t(c) & 7)));
  }
}
//-----------------------------------------------------------------------------
inline void invertBitRange(void * ptr,uintptr_t n,uintptr_t c)
{
  invertBitRangeT<void>(ptr,n,c);
}
//-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif
