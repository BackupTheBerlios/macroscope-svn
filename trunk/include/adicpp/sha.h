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
#ifndef _sha_H_
//---------------------------------------------------------------------------
#define _sha_H_
//---------------------------------------------------------------------------
extern "C" {
//---------------------------------------------------------------------------
#ifndef __BEGIN_DECLS
#define __BEGIN_DECLS extern "C" {
#endif
#ifndef __END_DECLS
#define __END_DECLS }
#endif
//---------------------------------------------------------------------------
#if HAVE_SHA256_H
#include <sha256.h>
#else
#include <adicpp/sha/sha256.h>
#endif
//---------------------------------------------------------------------------
#undef __BEGIN_DECLS
#undef __END_DECLS
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
inline void sha256(const void * src,uintptr_t len,uint8_t sha[32])
{
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx,(const unsigned char *) src,len);
  SHA256_Final(sha,&ctx);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SHA256 : private SHA256_CTX {
  public:
    ~SHA256();
    SHA256();

    SHA256 & init();
    SHA256 & update(const void * src,uintptr_t len);
    SHA256 & final();
    SHA256 & make(const void * src,uintptr_t len);

    const uint8_t * sha256() const;
  protected:
    uint8_t sha256_[32];
  private:
};
//---------------------------------------------------------------------------
inline SHA256::~SHA256()
{
}
//---------------------------------------------------------------------------
inline SHA256::SHA256()
{
}
//---------------------------------------------------------------------------
inline SHA256 & SHA256::init()
{
  SHA256_Init(this);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256 & SHA256::update(const void * src,uintptr_t len)
{
  SHA256_Update(this,(const unsigned char *) src,len);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256 & SHA256::final()
{
  SHA256_Final(sha256_,this);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256 & SHA256::make(const void * src,uintptr_t len)
{
  return init().update(src,len).final();
}
//---------------------------------------------------------------------------
inline const uint8_t * SHA256::sha256() const
{
  return sha256_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SHA256Cryptor : private SHA256 {
  public:
    ~SHA256Cryptor();
    SHA256Cryptor();

    SHA256Cryptor & init(const uint8_t sha256[32]);
    SHA256Cryptor & init(const void * key,uintptr_t len);
    SHA256Cryptor & crypt(void * data,uintptr_t len);
    SHA256Cryptor & crypt(void * dst,const void * src,uintptr_t len);

    const uintptr_t & threshold() const;
    SHA256Cryptor & threshold(uintptr_t threshold);
  protected:
  private:
    uintptr_t cursor_;
    uintptr_t count_;
    uintptr_t threshold_;
};
//---------------------------------------------------------------------------
inline SHA256Cryptor::~SHA256Cryptor()
{
}
//---------------------------------------------------------------------------
inline SHA256Cryptor::SHA256Cryptor() : threshold_(1024 * 1024)
{
}
//---------------------------------------------------------------------------
inline SHA256Cryptor & SHA256Cryptor::init(const uint8_t sha256[32])
{
  memcpy(sha256_,sha256,sizeof(sha256_));
  cursor_ = 0;
  count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Cryptor & SHA256Cryptor::init(const void * key,uintptr_t len)
{
  make(key,len);
  cursor_ = 0;
  count_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
#ifndef __BCPLUSPLUS__
//---------------------------------------------------------------------------
inline SHA256Cryptor & SHA256Cryptor::crypt(void * data,uintptr_t len)
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
inline SHA256Cryptor & SHA256Cryptor::crypt(void * dst,const void * src,uintptr_t len)
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
inline const uintptr_t & SHA256Cryptor::threshold() const
{
  return threshold_;
}
//---------------------------------------------------------------------------
inline SHA256Cryptor & SHA256Cryptor::threshold(uintptr_t threshold)
{
  threshold_ = threshold;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class SHA256Filter {
  public:
    ~SHA256Filter();
    SHA256Filter();

    SHA256Filter & init(const uint8_t sha256[32]);
    SHA256Filter & init(const void * key,uintptr_t len);
    SHA256Filter & encrypt(void * data,uintptr_t len);
    SHA256Filter & encrypt(void * dst,const void * src,uintptr_t len);
    SHA256Filter & decrypt(void * data,uintptr_t len);
    SHA256Filter & decrypt(void * dst,const void * src,uintptr_t len);

    const uintptr_t & threshold() const;
    SHA256Filter & threshold(uintptr_t threshold);

    SHA256Filter & active(bool active);
    const bool & active() const;
  protected:
  private:
    SHA256Cryptor encryptor_;
    SHA256Cryptor decryptor_;
    bool active_;
};
//---------------------------------------------------------------------------
inline SHA256Filter::~SHA256Filter()
{
}
//---------------------------------------------------------------------------
inline SHA256Filter::SHA256Filter() : active_(false)
{
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::init(const uint8_t sha256[32])
{
  encryptor_.init(sha256);
  decryptor_.init(sha256);
  active_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::init(const void * key,uintptr_t len)
{
  encryptor_.init(key,len);
  decryptor_.init(key,len);
  active_ = true;
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::encrypt(void * data,uintptr_t len)
{
  encryptor_.crypt(data,len);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::encrypt(void * dst,const void * src,uintptr_t len)
{
  if( active_ ) encryptor_.crypt(dst,src,len); else memcpy(dst,src,len);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::decrypt(void * data,uintptr_t len)
{
  decryptor_.crypt(data,len);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::decrypt(void * dst,const void * src,uintptr_t len)
{
  if( active_ ) decryptor_.crypt(dst,src,len); else memcpy(dst,src,len);
  return *this;
}
//---------------------------------------------------------------------------
inline const uintptr_t & SHA256Filter::threshold() const
{
  return encryptor_.threshold();
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::threshold(uintptr_t threshold)
{
  encryptor_.threshold(threshold);
  decryptor_.threshold(threshold);
  return *this;
}
//---------------------------------------------------------------------------
inline SHA256Filter & SHA256Filter::active(bool active)
{
  active_ = active_;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & SHA256Filter::active() const
{
  return active_;
}
//---------------------------------------------------------------------------
} //namespace ksys
//---------------------------------------------------------------------------
#endif /* _sha_H_ */
//---------------------------------------------------------------------------
