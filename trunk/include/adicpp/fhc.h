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
#ifndef fhcH
#define fhcH
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if defined(__CYGWIN__)
#define O_EXLOCK _FEXLOCK
#define O_SHLOCK _FSHLOCK
#endif
#ifndef O_EXLOCK
#define O_EXLOCK O_EXCL
#endif
//---------------------------------------------------------------------------
#if !defined(__WIN32__) && !defined(__WIN64__)
const int INVALID_HANDLE_VALUE  = -1;
#endif
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FileHandleContainer {
  public:
    ~FileHandleContainer();
    FileHandleContainer(const utf8::String & fileName = utf8::String());

    bool                  isOpen() const;
    FileHandleContainer & open();
    bool                  tryOpen(bool createIfNotExist = false);
    FileHandleContainer & close();
    FileHandleContainer & seek(uint64_t pos);
    uint64_t              size() const;
    FileHandleContainer & resize(uint64_t nSize);
    uint64_t              tell() const;
    int64_t               read(void * buf, uint64_t size);
    int64_t               write(const void * buf, uint64_t size);
    int64_t               read(uint64_t pos, void * buf, uint64_t size);
    int64_t               write(uint64_t pos, const void * buf, uint64_t size);
    FileHandleContainer & readBuffer(void * buf, uint64_t size);
    FileHandleContainer & writeBuffer(const void * buf, uint64_t size);
    FileHandleContainer & readBuffer(uint64_t pos, void * buf, uint64_t size);
    FileHandleContainer & writeBuffer(uint64_t pos, const void * buf, uint64_t size);

    bool                  tryRDLock(uint64_t pos, uint64_t size);
    bool                  tryWRLock(uint64_t pos, uint64_t size);
    FileHandleContainer & rdLock(uint64_t pos, uint64_t size);
    FileHandleContainer & wrLock(uint64_t pos, uint64_t size);
    FileHandleContainer & unLock(uint64_t pos, uint64_t size);

    uintptr_t             gets(AutoPtr< char> & p);

    const utf8::String &  fileName() const;
    FileHandleContainer & fileName(const utf8::String & name);
    bool exclusive() const;
    FileHandleContainer & exclusive(bool v);
    bool removeAfterClose() const;
    FileHandleContainer & removeAfterClose(bool v);
    bool readOnly() const;
    FileHandleContainer & readOnly(bool v);
    bool createIfNotExist() const;
    FileHandleContainer & createIfNotExist(bool v);

    FileHandleContainer & operator <<(const char * text);
    FileHandleContainer & operator <<(const utf8::String & s);
  protected:
#if _MSC_VER
#pragma warning(push,3)
#endif
#if defined(__WIN32__) || defined(__WIN64__)
    union int64 {
        int64_t a;
        struct {
            ULONG lo;
            LONG  hi;
        };
    };
    union uint64 {
        uint64_t  a;
        struct {
            ULONG lo;
            ULONG hi;
        };
    };
    HANDLE        handle_;
#else
    int           handle_;
#endif
    utf8::String  fileName_;
    struct {
      uint8_t exclusive_        : 1;
      uint8_t removeAfterClose_ : 1;
      uint8_t readOnly_         : 1;
      uint8_t createIfNotExist_ : 1;
    };
#if _MSC_VER
#pragma warning(pop)
#endif
  private:
};
//---------------------------------------------------------------------------
inline bool FileHandleContainer::isOpen() const
{
  return handle_ != INVALID_HANDLE_VALUE;
}
//---------------------------------------------------------------------------
inline const utf8::String & FileHandleContainer::fileName() const
{
  return fileName_;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::fileName(const utf8::String & name)
{
  fileName_ = name;
  return *this;
}
//---------------------------------------------------------------------------
inline bool FileHandleContainer::exclusive() const
{
  return exclusive_ != 0;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::exclusive(bool v)
{
  exclusive_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool FileHandleContainer::removeAfterClose() const
{
  return removeAfterClose_ != 0;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::removeAfterClose(bool v)
{
  removeAfterClose_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool FileHandleContainer::readOnly() const
{
  return readOnly_ != 0;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::readOnly(bool v)
{
  readOnly_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline bool FileHandleContainer::createIfNotExist() const
{
  return createIfNotExist_ != 0;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::createIfNotExist(bool v)
{
  createIfNotExist_ = v;
  return *this;
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::operator << (const char * text)
{
  return writeBuffer(text, (uintptr_t) ::strlen(text));
}
//---------------------------------------------------------------------------
inline FileHandleContainer & FileHandleContainer::operator << (const utf8::String & s)
{
  return writeBuffer(s.c_str(),s.size());
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EFileError : public Exception {
  public:
    EFileError(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
class EFileEOF : public Exception {
  public:
    EFileEOF(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
class EDiskFull : public Exception {
  public:
    EDiskFull(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
class EFLock : public Exception {
  public:
    EFLock(int32_t code, const utf8::String & what);
};
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
#endif
