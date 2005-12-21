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
#ifndef _lzo_H_
#define _lzo_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class LZO1X {
  friend void initialize();
  public:
    ~LZO1X();
    LZO1X();

    uint8_t * rBuf() const;
    LZO1X & rBufPos(uint32_t rBufPos);
    const uint32_t & rBufPos() const;
    LZO1X & rBufSize(uint32_t rBufSize);
    const uint32_t & rBufSize() const;
    uint32_t rBufSpace() const;
    int64_t read(void * buf,uint64_t len);

    uint8_t * wBuf() const;
    LZO1X & wBufPos(uint32_t wBufPos);
    const uint32_t & wBufPos() const;
    LZO1X & wBufSize(uint32_t wBufSize);
    const uint32_t & wBufSize() const;
    uint32_t wBufSpace() const;
    int64_t write(const void * buf,uint64_t len);

    LZO1X & level(uintptr_t lvl);
    const uint8_t & level() const;

    LZO1X & active(bool active);
    const bool & active() const;

    LZO1X & compress(AutoPtr<uint8_t> & buf);
    LZO1X & decompress(AutoPtr<uint8_t> & buf,uint32_t srcLen);
  protected:
  private:
    AutoPtr<uint8_t> rBuf_;
    AutoPtr<uint8_t> wBuf_;
    AutoPtr<uint8_t> wWrkMem_;
    uint32_t rBufPos_;
    uint32_t rBufSize_;
    uint32_t wBufPos_;
    uint32_t wBufSize_;
    uint8_t level_;
    bool active_;

    static void initialize();
};
//---------------------------------------------------------------------------
inline LZO1X::~LZO1X()
{
}
//---------------------------------------------------------------------------
inline LZO1X::LZO1X() : wBufSize_(256 * 1024), level_(1), active_(false)
{
}
//---------------------------------------------------------------------------
inline uint8_t * LZO1X::rBuf() const
{
  return rBuf_.ptr();
}
//---------------------------------------------------------------------------
inline LZO1X & LZO1X::rBufPos(uint32_t rBufPos)
{
  rBufPos_ = rBufPos;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint32_t & LZO1X::rBufPos() const
{
  return rBufPos_;
}
//---------------------------------------------------------------------------
inline LZO1X & LZO1X::rBufSize(uint32_t rBufSize)
{
  rBuf_.realloc(rBufSize);
  rBufSize_ = rBufSize;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint32_t & LZO1X::rBufSize() const
{
  return rBufSize_;
}
//---------------------------------------------------------------------------
inline uint32_t LZO1X::rBufSpace() const
{
  return rBufSize_ - rBufPos_;
}
//---------------------------------------------------------------------------
inline int64_t LZO1X::read(void * buf,uint64_t len)
{
  if( rBufSpace() < len ) len = rBufSpace();
  memcpy(buf,rBuf_.ptr() + rBufPos_,(size_t) len);
  rBufPos_ += (uint32_t) len;
  return len;
}
//---------------------------------------------------------------------------
inline uint8_t * LZO1X::wBuf() const
{
  return wBuf_.ptr();
}
//---------------------------------------------------------------------------
inline LZO1X & LZO1X::wBufPos(uint32_t wBufPos)
{
  wBufPos_ = wBufPos;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint32_t & LZO1X::wBufPos() const
{
  return wBufPos_;
}
//---------------------------------------------------------------------------
inline LZO1X & LZO1X::wBufSize(uint32_t wBufSize)
{
  wBufSize_ = (uintptr_t) wBufSize > 1024 * 1024 * 1024 ? 1024 * 1024 * 1024 : wBufSize;
  return *this;
}
//---------------------------------------------------------------------------
inline const uint32_t & LZO1X::wBufSize() const
{
  return wBufSize_;
}
//---------------------------------------------------------------------------
inline uint32_t LZO1X::wBufSpace() const
{
  return wBufSize_ - wBufPos_;
}
//---------------------------------------------------------------------------
inline int64_t LZO1X::write(const void * buf,uint64_t len)
{
  if( wBufSpace() < len ) len = wBufSpace();
  memcpy(wBuf_.ptr() + wBufPos_ + sizeof(int32_t) * 2,buf,(size_t) len);
  wBufPos_ += (uint32_t) len;
  return len;
}
//---------------------------------------------------------------------------
inline LZO1X & LZO1X::level(uintptr_t lvl)
{
  level_ = uint8_t(lvl > 9 ? 9 : lvl < 1 ? 1 : lvl);
  return *this;
}
//---------------------------------------------------------------------------
inline const uint8_t & LZO1X::level() const
{
  return level_;
}
//---------------------------------------------------------------------------
inline const bool & LZO1X::active() const
{
  return active_;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif /* _lzo_H_ */
