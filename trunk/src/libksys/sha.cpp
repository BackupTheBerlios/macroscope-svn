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
#if HAVE_SHA256_H
#include <sha256.h>
#else
#include <adicpp/sha/sha256.h>
#if defined(__WIN32__) || defined(__WIN64__)
#include "sha256c.c"
#endif
#endif
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
#ifdef __BCPLUSPLUS__
//---------------------------------------------------------------------------
SHA256Cryptor & SHA256Cryptor::crypt(void * data,uintptr_t len)
{
  while( len-- > 0 ){
    *(uint8_t *) data ^= sha256_[cursor_];
    data = (uint8_t *) data + 1;
    count_++;
    if( ++cursor_ >= sizeof(sha256_) ){
      if( count_ >= threshold_ ){
        make(sha256_,sizeof(sha256_));
        count_ = 0;
      }
      cursor_ = 0;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
SHA256Cryptor & SHA256Cryptor::crypt(void * dst,const void * src,uintptr_t len)
{
  while( len-- > 0 ){
    *(uint8_t *) dst = *(uint8_t *) src ^ sha256_[cursor_];
    dst = (uint8_t *) dst + 1;
    src = (uint8_t *) src + 1;
    count_++;
    if( ++cursor_ >= sizeof(sha256_) ){
      if( count_ >= threshold_ ){
        make(sha256_,sizeof(sha256_));
        count_ = 0;
      }
      cursor_ = 0;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
