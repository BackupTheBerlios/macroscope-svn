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
namespace ksys {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void Exception::afterConstruction()
{
  if( !heap_ )
    newObjectV1C2<Exception>(
      EINVAL,
      __PRETTY_FUNCTION__ + utf8::String(" ") +
      "Exception must be allocated dynamicaly only via newObject function(s)."
    )->throwSP();
}
//---------------------------------------------------------------------------
/*Exception * Exception::newObject()
{
  ksys::AutoPtr<Exception> safe((Exception *) ksys::kmalloc(sizeof(Exception)));
  new (safe) Exception;
  KsysObjectActions::afterConstruction(safe.ptr());
  return safe.ptr(NULL);
}
//---------------------------------------------------------------------------
Exception * Exception::newObject(int32_t code,const utf8::String & what)
{
  AutoPtr<Exception> e(Exception::newObject());
  e->codes().add(code);
  e->whats().add(what);
  return e.ptr(NULL);
}
//---------------------------------------------------------------------------
Exception * Exception::newObject(int32_t code,const char * what)
{
  AutoPtr<Exception> e(Exception::newObject());
  e->codes().add(code);
  e->whats().add(what);
  return e.ptr(NULL);
}*/
//---------------------------------------------------------------------------
Exception::~Exception()
{
}
//---------------------------------------------------------------------------
Exception::Exception() : refCount_(0), stackBackTrace_(true)
{
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const utf8::String & what) : refCount_(0), stackBackTrace_(true)
{
  codes_.add(code);
  whats_.add(what);
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const char * what) : refCount_(0), stackBackTrace_(true)
{
  codes_.add(code);
  whats_.add(what);
}
//---------------------------------------------------------------------------
bool Exception::isFatalError() const
{
  return false;
}
//---------------------------------------------------------------------------
int32_t Exception::code() const
{
  return codes_.count() > 0 ? codes_[(uintptr_t) 0] : 0;
}
//---------------------------------------------------------------------------
const utf8::String Exception::what() const
{
  return whats_.count() > 0 ? whats_[(uintptr_t) 0] : utf8::String();
}
//---------------------------------------------------------------------------
void Exception::throwSP()
{
#if __GNUG__ || (!defined(NDEBUG) && (defined(__WIN32__) || defined(__WIN64__)))
  if( stdErr.debugLevel(128) && stackBackTrace_ && ksys::stackBackTrace ){
    try {
      utf8::String::Stream stackTrace;
//      fprintf(stderr,"%s %d\n",__FILE__,__LINE__);
      stackTrace << "\n" <<
#if __GNUG__
        getBackTrace(1)
#else
        getBackTrace(5)
#endif
      ;
      if( stackTrace.count() > 1 ) stdErr.debug(128,stackTrace);
    }
    catch( ... ){
    }
  }
#endif
  refCount_ = 0;
  throw ExceptionSP(this);
}
//---------------------------------------------------------------------------
utf8::String Exception::stdError(utf8::String::Stream * s) const
{
  utf8::String::Stream stream;
  if( s == NULL ) s = &stream;
  for( uintptr_t i = 0; i < whats_.count(); i++ ){
    if( codes_[0] == 0 ) continue;
    intmax_t a;
    utf8::String serr(strError(codes_[i]));
    if( !utf8::tryStr2Int(serr,a) ){
      if( codes_[i] >= errorOffset ) *s << codes_[i] - errorOffset; else *s << codes_[i];
      *s << " ";
    }
    if( serr.strlen() > 0 ) *s << serr << " ";
    *s << whats_[i] << "\n";
  }
  return s == &stream ? s->string() : utf8::String();
}
//---------------------------------------------------------------------------
const Exception & Exception::writeStdError(LogFile * log) const
{
  if( stdErr.debugLevel(9) ){
    utf8::String::Stream s;
    stdError(&s);
    (log != NULL ? log : &stdErr)->debug(9,s);
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

