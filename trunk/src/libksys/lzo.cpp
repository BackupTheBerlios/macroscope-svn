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
extern "C" {
//---------------------------------------------------------------------------
#if HAVE_CONFIG_H
#define LZO_HAVE_CONFIG_H 1
#endif
#if defined(_DEBUG) || defined(DEBUG)
#define LZO_DEBUG 1
#endif
#if HAVE_LZO_ASM_H
#include <lzo_asm.h>
#endif
#if HAVE_LZO1X_H
#include <lzo1x.h>
#else
#include <adicpp/lzo/lzo1x.h>
//#include "lzo1x_9x.c"
//#include "lzo1x_d1.c"
//#include "lzo1x_d2.c"
//#include "lzo1x_d3.c"
//#include "lzo1x_o.c"
//#include "lzo_init.c"
#endif
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void LZO1X::initialize()
{
  int r = lzo_init();
  assert( r == LZO_E_OK );
  if( r != LZO_E_OK )
    throw ExceptionSP(
      new Exception(-1,utf8::string(__PRETTY_FUNCTION__))
    );
}
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8013 -w-8004
#endif
//---------------------------------------------------------------------------
LZO1X & LZO1X::compress(AutoPtr<uint8_t> & buf)
{
  buf.realloc(wBufPos_ + wBufPos_ / 16 + 64 + 3 + sizeof(int32_t) * 2);
  lzo_uint dst_len = 0;
  int r = lzo1x_999_compress_level(
    (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t) * 2),
    wBufPos_,
    (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
    &dst_len,
    (lzo_voidp) wWrkMem_.ptr(),
    NULL,
    0,
    NULL,
    level_
  );
  assert( r == LZO_E_OK );
  if( r != LZO_E_OK ) throw ExceptionSP(new Exception(-1,utf8::string(__PRETTY_FUNCTION__)));
  lzo_uint orig_len = wBufPos_;
  r = lzo1x_optimize(
    (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
    dst_len,
    (lzo_bytep) (wBuf_.ptr() + sizeof(int32_t) * 2),
    &orig_len,
    NULL
  );
  assert( r == LZO_E_OK );
  ((int32_t *) wBuf_.ptr())[0] = dst_len;
  ((int32_t *) wBuf_.ptr())[1] = wBufPos_;
  ((int32_t *) buf.ptr())[0] = dst_len;
  ((int32_t *) buf.ptr())[1] = wBufPos_;
  return *this;
}
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
LZO1X & LZO1X::decompress(AutoPtr<uint8_t> & buf,uintptr_t srcLen)
{
  lzo_uint dst_len = rBufSize_;
  int r =
#if (defined(_DEBUG) || defined(DEBUG)) && HAVE_LZO1X_DECOMPRESS_SAFE
   lzo1x_decompress_safe(
#elif HAVE_LZO1X_DECOMPRESS_ASM_FAST
   lzo1x_decompress_asm_fast(
#elif HAVE_LZO1X_DECOMPRESS_ASM_FAST_SAFE
   lzo1x_decompress_asm_fast_safe(
#elif HAVE_LZO1X_DECOMPRESS_ASM
   lzo1x_decompress_asm(
#elif HAVE_LZO1X_DECOMPRESS_ASM_SAFE
   lzo1x_decompress_asm_safe(
#elif HAVE_LZO1X_DECOMPRESS
   lzo1x_decompress(
#elif HAVE_LZO1X_DECOMPRESS_SAFE
   lzo1x_decompress_safe(
#elif !HAVE_LIBLZO && !HAVE_LIBLZO2
   lzo1x_decompress(
#else
#error broken lzo library
#endif
    (const lzo_bytep) buf.ptr(),
    (lzo_uint) srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
  assert( r == LZO_E_OK && dst_len == rBufSize_ );
  if( r != LZO_E_OK || dst_len != rBufSize_ )
    throw ExceptionSP(
      new Exception(-1,utf8::string(__PRETTY_FUNCTION__))
    );
  return *this;
}
//---------------------------------------------------------------------------
LZO1X & LZO1X::active(bool active)
{
  active_ = active;
  if( active ){
    rBuf_.free();
    wBuf_.realloc(wBufSize_ + sizeof(int32_t) * 2);
    wWrkMem_.realloc(LZO1X_999_MEM_COMPRESS);
    wBufPos_ = 0;
    rBufPos_ = 0;
    rBufSize_ = 0;
    wBufPos_ = 0;
  }
  else {
    wWrkMem_.free();
    wBuf_.free();
    rBuf_.free();
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
