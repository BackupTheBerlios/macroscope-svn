/*-
 * Copyright 2009 Guram Dukashvili
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
#ifndef kstringH
#define kstringH
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
void kmfree(void * p);
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
const uint16_t cpAnsi = 0;
const uint16_t cpOem = 1;
const uint16_t cpMac = 2;
const uint16_t cpUtf16 = 1200;
const uint16_t cpUtf7 = 65000;
const uint16_t cpUtf8 = 65001;
const uint16_t cpUtf32 = 65002;
const uint16_t cpUnix = 65010;
//---------------------------------------------------------------------------
class String {
  public:
    ~String();
    String();
    String(const char * s);
    String(const String & s);
    String(uintptr_t v);

    String & operator = (const char * s);
    String & operator = (const String & s)
    {
      s.data_->addRef();
      data_->remRef();
      data_ = s.data_;
      return *this;
    }

    String operator + (const char * s) const;
    String operator + (const String & s) const;

    String & operator += (const char * s);
    String & operator += (const String & s);

    operator const char * () const;

    static String print(const char * format, ... );
    uintptr_t length() const;
    uintptr_t size() const;
    uintptr_t termCharSize() const;
    bool isNull() const;

    String changeCodePage(uintptr_t newCodePage) const;

    static String elapsedTime2Str(uint64_t time);
  protected:
    struct Data {
      volatile ilock_t refCount_;
      uint16_t codePage_;
      union {
        char ss_[1024];
        unsigned char us_[1024];
#if SIZEOF_WCHAR_T == 2
        wchar_t ws_[1024];
        uint32_t es_[1024];
#elif SIZEOF_WCHAR_T == 4
        uint16_t ws_[1024];
        wchar_t es_[1024];
#else
        uint16_t ws_[1024];
        uint32_t es_[1024];
#endif
      };

      Data * addRef()
      {
        interlockedIncrement(refCount_,1);
        return this;
      }

      void remRef()
      {
        if( interlockedIncrement(refCount_,-1) == 1 ) kmfree(this);
      }

    };

    Data * data_;
    static uint8_t null_[];

    String(Data * data) : data_(data) {}

    static Data * newData(uintptr_t codePage,uintptr_t size,ilock_t refCount = 1);
  private:
};
//---------------------------------------------------------------------------
String operator + (const char * s1,const String & s2);
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option pop
#endif
//---------------------------------------------------------------------------
#endif /* kstringH */
//---------------------------------------------------------------------------
