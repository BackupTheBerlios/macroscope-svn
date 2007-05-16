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
#ifndef _mstreamH_
#define _mstreamH_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
class MemoryStream {
    friend void initialize(int,char **);
    friend void cleanup();
  private:
    static void initialize();
    static void cleanup();
  protected:
    class Container {
      friend class MemoryStream;
      public:
        ~Container();
        Container() {}
        Container(uintptr_t count);
        Container(int32_t refCount,void * ptr,uintptr_t count);

        Container & addRef();
        Container & remRef();
      protected:
      private:
        union {
          void *          ptr_;
          char *          sptr_;
          unsigned char * uptr_;
        };
        uintptr_t count_;
        uintptr_t mcount_;
        volatile int32_t   refCount_;
    };

    mutable SPRC<Container> container_;
    uintptr_t pos_;
    
    static uint8_t    nullContainer_[];
    static Container &  nullContainer();
    
    MemoryStream(Container * container);
  public:
    ~MemoryStream();
    MemoryStream();
    MemoryStream(const MemoryStream & stream);

    MemoryStream &      operator =(const MemoryStream & stream);

    MemoryStream &      operator >>(char & n);
    MemoryStream &      operator >>(unsigned char & n);
    MemoryStream &      operator >>(short & n);
    MemoryStream &      operator >>(unsigned short & n);
#if !HAVE_INTPTR_T_AS_LONG
    MemoryStream &      operator >>(long & n);
    MemoryStream &      operator >>(unsigned long & n);
#endif
    MemoryStream &      operator >>(int & n);
    MemoryStream &      operator >>(unsigned int & n);
#if !HAVE_INTPTR_T_AS_INT
    MemoryStream &      operator >>(intptr_t & n);
    MemoryStream &      operator >>(uintptr_t & n);
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T
    MemoryStream &      operator >>(intmax_t & n);
    MemoryStream &      operator >>(uintmax_t & n);
#endif
#if !HAVE_INT64_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTPTR_T
    MemoryStream &      operator >>(int64_t & n);
    MemoryStream &      operator >>(uint64_t & n);
#endif
    MemoryStream &      operator >>(float & n);
    MemoryStream &      operator >>(double & n);
#if HAVE_LONG_DOUBLE
    MemoryStream &      operator >>(long double & n);
#endif
    MemoryStream &      operator >>(utf8::String & s);

    MemoryStream &      operator <<(char n);
    MemoryStream &      operator <<(unsigned char n);
    MemoryStream &      operator <<(short n);
    MemoryStream &      operator <<(unsigned short n);
#if !HAVE_INTPTR_T_AS_LONG
    MemoryStream &      operator <<(long n);
    MemoryStream &      operator <<(unsigned long n);
#endif
    MemoryStream &      operator <<(int n);
    MemoryStream &      operator <<(unsigned int n);
#if !HAVE_INTPTR_T_AS_INT
    MemoryStream &      operator <<(intptr_t n);
    MemoryStream &      operator <<(uintptr_t n);
#endif
#if !HAVE_INTPTR_T_AS_INTMAX_T
    MemoryStream &      operator <<(intmax_t n);
    MemoryStream &      operator <<(uintmax_t n);
#endif
#if !HAVE_INT64_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTPTR_T
    MemoryStream &      operator <<(int64_t n);
    MemoryStream &      operator <<(uint64_t n);
#endif
    MemoryStream &      operator <<(float n);
    MemoryStream &      operator <<(double n);
#if HAVE_LONG_DOUBLE
    MemoryStream &      operator <<(long double n);
#endif
    MemoryStream &      operator <<(const utf8::String & s);

    bool                bof() const;
    bool                eof() const;
    uintptr_t           count() const;
    uintptr_t           pos() const;

    MemoryStream &      clear();

    intptr_t            read(void * buffer, uintptr_t count);
    intptr_t            write(const void * buffer, uintptr_t count);
    MemoryStream &      readBuffer(void * buffer, uintptr_t count);
    MemoryStream &      writeBuffer(const void * buffer, uintptr_t count);

    MemoryStream &      resize(uintptr_t newSize);

    MemoryStream &      seek(uintptr_t newPos);

    MemoryStream        unique() const;

    void * raw() const;
    //    static MemoryStream reference(void * ptr,long count);
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline MemoryStream::Container::~Container()
{
  xfree(ptr_);
}
//---------------------------------------------------------------------------
inline MemoryStream::Container::Container(int32_t refCount, void * ptr, uintptr_t count)
  : ptr_(ptr),
    count_(count),
    mcount_(count),
    refCount_(refCount)
{
}
//---------------------------------------------------------------------------
inline MemoryStream::Container & MemoryStream::Container::addRef()
{
  interlockedIncrement(refCount_, 1);
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream::Container & MemoryStream::Container::remRef()
{
  if( interlockedIncrement(refCount_, -1) == 1 ) deleteObject(this);
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline MemoryStream::Container & MemoryStream::nullContainer()
{
  return *reinterpret_cast< Container *>(nullContainer_);
}
//---------------------------------------------------------------------------
inline MemoryStream::MemoryStream(Container * container) : container_(container)
{
}
//---------------------------------------------------------------------------
inline MemoryStream::~MemoryStream()
{
}
//---------------------------------------------------------------------------
inline MemoryStream::MemoryStream() : container_(&nullContainer()), pos_(0)
{
}
//---------------------------------------------------------------------------
inline MemoryStream::MemoryStream(const MemoryStream & stream) : container_(stream.container_)
{
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator =(const MemoryStream & stream)
{
  container_ = stream.container_;
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(char & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(unsigned char & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(short & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(unsigned short & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_LONG
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(long & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(unsigned long & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(int & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(unsigned int & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(intptr_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(uintptr_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(intmax_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(uintmax_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_INT64_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTPTR_T
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(int64_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(uint64_t & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(float & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(double & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator >>(long double & n)
{
  readBuffer(&n, sizeof(n));
  return *this;
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(char n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(unsigned char n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(short n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(unsigned short n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_LONG
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(long n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(unsigned long n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(int n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(unsigned int n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
inline MemoryStream & MemoryStream::operator <<(intptr_t n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(uintptr_t n)
{
  return writeBuffer(&n, sizeof(n));
}
#endif
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INTMAX_T
inline MemoryStream & MemoryStream::operator <<(intmax_t n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(uintmax_t n)
{
  return writeBuffer(&n, sizeof(n));
}
#endif
//---------------------------------------------------------------------------
#if !HAVE_INT64_T_AS_INTMAX_T && !HAVE_INT64_T_AS_INTPTR_T
inline MemoryStream & MemoryStream::operator <<(int64_t n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(uint64_t n)
{
  return writeBuffer(&n, sizeof(n));
}
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(float n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(double n)
{
  return writeBuffer(&n, sizeof(n));
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
inline MemoryStream & MemoryStream::operator <<(long double n)
{
  return writeBuffer(&n, sizeof(n));
}
#endif
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::operator <<(const utf8::String & s)
{
  return writeBuffer(s.c_str(), s.size() + 1);
}
//---------------------------------------------------------------------------
inline bool MemoryStream::bof() const
{
  return pos_ == 0;
}
//---------------------------------------------------------------------------
inline bool MemoryStream::eof() const
{
  return pos_ == container_->count_;
}
//---------------------------------------------------------------------------
inline uintptr_t MemoryStream::count() const
{
  return container_->count_;
}
//---------------------------------------------------------------------------
inline uintptr_t MemoryStream::pos() const
{
  return pos_;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::clear()
{
  container_ = &nullContainer();
  pos_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
inline intptr_t MemoryStream::read(void * buffer, uintptr_t count)
{
  if( pos_ + count > container_->count_ ) count = container_->count_ - pos_;
  memcpy(buffer, container_->uptr_ + pos_, count);
  pos_ += count;
  return count;
}
//---------------------------------------------------------------------------
inline intptr_t MemoryStream::write(const void * buffer, uintptr_t count)
{
  if( pos_ + count > container_->mcount_ ) resize(pos_ + count);
  memcpy(container_->uptr_ + pos_, buffer, count);
  pos_ += count;
  return count;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::writeBuffer(const void * buffer, uintptr_t count)
{
  write(buffer, count);
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream & MemoryStream::seek(uintptr_t newPos)
{
  if( newPos > container_->count_ ) resize(newPos);
  pos_ = newPos;
  return *this;
}
//---------------------------------------------------------------------------
inline MemoryStream MemoryStream::unique() const
{
  Container * container = newObjectV1<Container>(container_->count_);
  memcpy(container->ptr_, container_->ptr_, container_->count_);
  return container;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
inline void * MemoryStream::raw() const
{
  return container_->ptr_;
}
//---------------------------------------------------------------------------
class EMemoryStreamEOF : public Exception {
  public:
    EMemoryStreamEOF() {}
    EMemoryStreamEOF(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
class EMemoryStreamReadError : public Exception {
  public:
    EMemoryStreamReadError() {}
    EMemoryStreamReadError(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
}
//---------------------------------------------------------------------------
#endif
