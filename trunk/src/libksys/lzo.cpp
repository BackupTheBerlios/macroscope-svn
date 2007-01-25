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
#if (defined(_DEBUG) || defined(DEBUG)) && !defined(NDEBUG)
#define LZO_DEBUG 1
#endif
#if HAVE_LZO_LZO_ASM_H
#include <lzo/lzo_asm.h>
#elif HAVE_LZO_ASM_H
#include <lzo_asm.h>
#endif
#if HAVE_LZO_LZO1X_H
#include <lzo/lzo1x.h>
#elif HAVE_LZO1X_H
#include <lzo1x.h>
#else
#if HAVE_CONFIG_H
#define LZO_HAVE_CONFIG_H 1
#endif
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
  assert(r == LZO_E_OK);
  if( r != LZO_E_OK )
    newObjectV1C2<Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
}
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option push -w-8013 -w-8004
#endif
//---------------------------------------------------------------------------
LZO1X & LZO1X::compress(AutoPtr<uint8_t> & buf,uint8_t * & p,int32_t & len)
{
  int r = 0;
  lzo_uint dst_len = 0;
  if( level_ > 0 ){
    buf.realloc(wBufPos_ + wBufPos_ / 16 + 64 + 3 + sizeof(int32_t) * 2 + (crc_ != CRCNone) * sizeof(uint32_t));
    switch( method_ ){
      case LZO1X_1    :
        r = lzo1x_1_compress(
          (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
          wBufPos_,
          (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
          &dst_len,
          (lzo_voidp) wWrkMem_.ptr()
        );
        break;
      case LZO1X_1_11 :
        r = lzo1x_1_11_compress(
          (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
          wBufPos_,
          (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
          &dst_len,
          (lzo_voidp) wWrkMem_.ptr()
        );
        break;
      case LZO1X_1_12 :
        r = lzo1x_1_11_compress(
          (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
          wBufPos_,
          (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
          &dst_len,
          (lzo_voidp) wWrkMem_.ptr()
        );
        break;
      case LZO1X_1_15 :
        r = lzo1x_1_15_compress(
          (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
          wBufPos_,
          (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
          &dst_len,
          (lzo_voidp) wWrkMem_.ptr()
        );
        break;
      case LZO1X_999  :
        r = lzo1x_999_compress_level(
          (const lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
          wBufPos_,
          (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
          &dst_len,
          (lzo_voidp) wWrkMem_.ptr(),
          NULL,
          0,
          NULL,
          level_
        );
        break;
      default : assert( 0 );
    }
    assert( r == LZO_E_OK );
    if( r != LZO_E_OK )
      newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    if( dst_len >= wBufPos_ ) goto l1;
    if( optimize_ ){
      lzo_uint orig_len = wBufPos_;
      r = lzo1x_optimize(
        (lzo_bytep) (buf.ptr() + sizeof(int32_t) * 2),
        dst_len,
        (lzo_bytep) (wBuf_.ptr() + sizeof(int32_t)),
        &orig_len,
        NULL
      );
      assert( r == LZO_E_OK );
    }
    if( crc_ != CRCNone ){
      lzo_uint32 checksum = 0;
      if( crc_ == CRC32 ){
        checksum = lzo_crc32(0,NULL,0);
        checksum = lzo_crc32(checksum,buf.ptr() + sizeof(int32_t) * 2,dst_len);
      }
      else if( crc_ == ADLER32 ){
        checksum = lzo_adler32(0,NULL,0);
        checksum = lzo_adler32(checksum,buf.ptr() + sizeof(int32_t) * 2,dst_len);
      }
      *(uint32_t *)(buf.ptr() + sizeof(int32_t) * 2 + dst_len) = checksum;
    }
    dst_len += sizeof(int32_t) * 2 + (crc_ != CRCNone) * sizeof(uint32_t);
    ((int32_t *) buf.ptr())[0] = (int32_t) dst_len;
    ((int32_t *) buf.ptr())[1] = wBufPos_;
    p = buf;
    len = (int32_t) dst_len;
  }
  else {
l1: ((int32_t *) wBuf_.ptr())[0] = -int32_t(wBufPos_);
    p = wBuf_;
    len = (int32_t) (wBufPos_ + sizeof(int32_t));
  }
  return *this;
}
//---------------------------------------------------------------------------
#if __BCPLUSPLUS__
#pragma option pop
#endif
//---------------------------------------------------------------------------
LZO1X & LZO1X::decompress(AutoPtr<uint8_t> & buf)
{
  union {
    int32_t * i32;
    uint32_t * u32;
    uint8_t * i8;
  };
  i8 = buf;
  int32_t a = (int32_t) (i32[0] - sizeof(int32_t) * 2 - (crc_ != CRCNone) * sizeof(uint32_t));
  if( crc_ != CRCNone && a > 0 ){
    lzo_uint32 checksum = 0;
    if( crc_ == CRC32 ){
      checksum = lzo_crc32(0,NULL,0);
      checksum = lzo_crc32(checksum,i8 + sizeof(int32_t) * 2,a);
    }
    else if( crc_ == ADLER32 ){
      checksum = lzo_adler32(0,NULL,0);
      checksum = lzo_adler32(checksum,i8 + sizeof(int32_t) * 2,a);
    }
    if( *(uint32_t *) (i8 + sizeof(int32_t) * 2 + a) != checksum ) a = -a;
  }
  if( a <= 0 || i32[1] <= 0 )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  rBufSize(i32[1]);
  lzo_uint srcLen = a, dst_len = i32[1];
#if HAVE_LZO1X_DECOMPRESS_ASM_FAST_SAFE
  int r = lzo1x_decompress_asm_fast_safe(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
#elif HAVE_LZO1X_DECOMPRESS_ASM_SAFE
  int r = lzo1x_decompress_asm_safe(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
#elif HAVE_LZO1X_DECOMPRESS_SAFE
  int r = lzo1x_decompress_safe(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
#elif HAVE_LZO1X_DECOMPRESS_ASM_FAST
  int r = lzo1x_decompress_asm_fast(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
#elif HAVE_LZO1X_DECOMPRESS_ASM
  int r = lzo1x_decompress_asm(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
//#elif HAVE_LZO1X_DECOMPRESS
#else
  int r = lzo1x_decompress(
    (const lzo_bytep) (i8 + sizeof(int32_t) * 2),
    srcLen,
    (lzo_bytep) rBuf_.ptr(),
    &dst_len,
    NULL
  );
//#else
//#error broken lzo library
#endif
  assert( r == LZO_E_OK && dst_len == rBufSize_ );
  if( r != LZO_E_OK || dst_len != rBufSize_ )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
LZO1X & LZO1X::active(bool active)
{
  if( active ){
    rBuf_.free();
    wBuf_.realloc(wBufSize_ + sizeof(int32_t));
    static const size_t wrksz[] = {
      LZO1X_1_MEM_COMPRESS,
      LZO1X_1_11_MEM_COMPRESS,
      LZO1X_1_12_MEM_COMPRESS,
      LZO1X_1_15_MEM_COMPRESS,
      LZO1X_999_MEM_COMPRESS
    };
    wWrkMem_.realloc(wrksz[method_]);
  }
  else{
    wWrkMem_.free();
    wBuf_.free();
    rBuf_.free();
  }
  wBufPos_ = 0;
  rBufPos_ = 0;
  rBufSize_ = 0;
  wBufPos_ = 0;
  active_ = active;
  return *this;
}
//---------------------------------------------------------------------------
uint32_t adler32(uint32_t adler,const void * buf, uintptr_t len)
{
  return lzo_adler32(adler,(const lzo_bytep) buf,(lzo_uint) len);
}
//---------------------------------------------------------------------------
uint32_t crc32(uint32_t c,const void * buf,uintptr_t len)
{
  return lzo_crc32(c,(const lzo_bytep) buf,(lzo_uint) len);
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
