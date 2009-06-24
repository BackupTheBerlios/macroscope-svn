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
#ifndef kexcpH
#define kexcpH
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
//---------------------------------------------------------------------------
namespace kvm {
//---------------------------------------------------------------------------
#define throwException(code,text) \
  throw kvm::Exception(code,kvm::String(text) + ", " + __FILE__ + ", " + kvm::String(__LINE__),false)
#define throwExceptionW(code,text) \
  throw kvm::Exception(code,kvm::String(text) + ", " + __FILE__ + ", " + kvm::String(__LINE__),true)
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Exception : public Object {
  public:
    virtual ~Exception();
    Exception();
    Exception(int32_t code,const String & what,bool winError = false);

    int32_t & code(uintptr_t i = 0) const;
    const String & what(uintptr_t i = 0) const;
    bool searchCode(int32_t code) const;
    bool searchCode(int32_t code1,int32_t code2) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5,int32_t code6) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7,int32_t code8) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7,int32_t code8,int32_t code9) const;

    static int32_t mapWinErr2Std(int32_t e);
    static int32_t mapStdErr2Win(int32_t e);

    Exception & addError(int32_t code,const String & what);
  protected:
    class Error {
      public:
        virtual ~Error() {}
        Error(int32_t code,const String & what) : code_(code), what_(what) {}

        int32_t code_;
        String what_;
    };
    mutable Array<Error> errors_;
  private:
    //Exception(const Exception &){}
    //void operator = (const Exception &){}
};
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2) const
{
  return searchCode(code1) || searchCode(code2);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3) const
{
  return searchCode(code1,code2) || searchCode(code3);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3, int32_t code4) const
{
  return searchCode(code1,code2,code3) || searchCode(code4);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3,int32_t code4,int32_t code5) const
{
  return searchCode(code1,code2,code3,code4) || searchCode(code5);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3,int32_t code4,int32_t code5,int32_t code6) const
{
  return searchCode(code1,code2,code3,code4,code5) || searchCode(code6);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7) const
{
  return searchCode(code1,code2,code3,code4,code5,code6) || searchCode(code7);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7,int32_t code8) const
{
  return searchCode(code1,code2,code3,code4,code5,code6,code7) || searchCode(code8);
}
//---------------------------------------------------------------------------
inline bool Exception::searchCode(int32_t code1, int32_t code2, int32_t code3,int32_t code4,int32_t code5,int32_t code6,int32_t code7,int32_t code8,int32_t code9) const
{
  return searchCode(code1,code2,code3,code4,code5,code6,code7,code8) || searchCode(code9);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EOutOfMemory : public Exception {
  public:
    EOutOfMemory() {}
    EOutOfMemory(int32_t code,const String what);
};
//---------------------------------------------------------------------------
inline EOutOfMemory::EOutOfMemory(int32_t code,const String what)
  : Exception(code, what)
{
}
//---------------------------------------------------------------------------
} // namespace kvm
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option pop
#endif
//---------------------------------------------------------------------------
#endif /* excpH */
//---------------------------------------------------------------------------
