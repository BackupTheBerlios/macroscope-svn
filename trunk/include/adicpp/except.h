/*-
 * Copyright 2005-2007 Guram Dukashvili
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
#ifndef _Exception_H_
#define _Exception_H_
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
#if defined(__WIN32__) || defined(__WIN64__)
const int errorOffset = 5000;
inline int32_t oserror(){ return GetLastError(); }
inline void oserror(int32_t err){ SetLastError(err); }
#else
const int errorOffset = 0;
inline int32_t oserror(){ return errno; }
inline void oserror(int32_t err){ errno = err; }
#endif
//---------------------------------------------------------------------------
class LogFile;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Exception : virtual public Object {
  public:
    virtual ~Exception();
    Exception();
    Exception(int32_t code,const utf8::String & what);
    Exception(int32_t code,const char * what);

    Exception & addRef();
    Exception & remRef();

    int32_t & code(uintptr_t i = 0) const;
    utf8::String & what(uintptr_t i = 0) const;
    bool searchCode(int32_t code) const;
    bool searchCode(int32_t code1,int32_t code2) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5) const;
    bool searchCode(int32_t code1,int32_t code2,int32_t code3,int32_t code4,int32_t code5,int32_t code6) const;

//    static Exception * newObject();
//    static Exception * newObject(int32_t code,const char * what);
//    static Exception * newObject(int32_t code,const utf8::String & what);

    virtual utf8::String stdError(utf8::String::Stream * s = NULL) const;
    virtual const Exception & writeStdError(LogFile * log = NULL) const;
    virtual bool isFatalError() const;
    virtual void DECLSPEC_NORETURN throwSP() GNUG_NORETURN;

    const bool & stackBackTrace() const;
    Exception & stackBackTrace(bool v);

    Exception & clearError(uintptr_t i = ~uintptr_t(0));
    Exception & addError(int32_t code,const utf8::String & what);
    Exception & addError(int32_t code,const char * what);
  protected:
    class Error {
      public:
        ~Error() {}
        Error(int32_t code,const utf8::String & what) : code_(code), what_(what) {}
        Error(int32_t code,const char * what) : code_(code), what_(what) {}
	
        int32_t code_;
        utf8::String what_;
    };
    mutable Array<Error> errors_;
    mutable volatile int32_t refCount_;
    bool stackBackTrace_;
  private:
    Exception(const Exception &){}
    void operator = (const Exception &){}

    void afterConstruction();
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
inline Exception & Exception::addRef()
{
  interlockedIncrement(refCount_, 1);
  return *this;
}
//---------------------------------------------------------------------------
inline Exception & Exception::remRef()
{
  if( interlockedIncrement(refCount_, -1) == 1 ) deleteObject(this);
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & Exception::stackBackTrace() const
{
  return stackBackTrace_;
}
//---------------------------------------------------------------------------
inline Exception & Exception::stackBackTrace(bool v)
{
  stackBackTrace_ = v;
  return *this;
}
//---------------------------------------------------------------------------
typedef SPRC<Exception>
//SP<
//  Exception,
//  RefCounted<Exception>,
//  NoCheck<Exception>,
//  DirectAccess<Exception>,
//  DefaultStorage<Exception>
//>
ExceptionSP;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EOutOfMemory : public Exception {
  public:
    EOutOfMemory() {}
    EOutOfMemory(int32_t code,const char * what);
    EOutOfMemory(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EOutOfMemory::EOutOfMemory(int32_t code,const char * what)
  : Exception(code, what)
{
}
//---------------------------------------------------------------------------
inline EOutOfMemory::EOutOfMemory(int32_t code,const utf8::String & what)
  : Exception(code, what)
{
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class EStr2Scalar : public ksys::Exception {
  public:
    EStr2Scalar() {}
    EStr2Scalar(int32_t code,const utf8::String & what);
};
//---------------------------------------------------------------------------
inline EStr2Scalar::EStr2Scalar(int32_t code,const utf8::String & what) :
  ksys::Exception(code,what)
{
}
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
#endif /* _Exception_H_ */
