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
  if( !object_.heap_ )
    newObjectV1C2<Exception>(
      EINVAL,
      __PRETTY_FUNCTION__ + utf8::String(" ") +
      "Exception must be allocated dynamicaly only via newObject function(s)."
    )->throwSP();
}
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
  addError(code,what);
}
//---------------------------------------------------------------------------
Exception::Exception(int32_t code, const char * what) : refCount_(0), stackBackTrace_(true)
{
  addError(code,what);
}
//---------------------------------------------------------------------------
bool Exception::isFatalError() const
{
  return false;
}
//---------------------------------------------------------------------------
Exception & Exception::clearError(uintptr_t i)
{
  if( i == ~uintptr_t(0) ){
    errors_.clear();
  }
  else {
    errors_.remove(i);
  }
  return *this;
}
//---------------------------------------------------------------------------
Exception & Exception::addError(int32_t code,const utf8::String & what)
{
  errors_.add(Error(code,what));
  return *this;
}
//---------------------------------------------------------------------------
Exception & Exception::addError(int32_t code,const char * what)
{
  errors_.add(Error(code,what));
  return *this;
}
//---------------------------------------------------------------------------
bool Exception::searchCode(int32_t code) const
{
  intptr_t i;
  for( i = errors_.count() - 1; i >= 0 && errors_[i].code_ != code; i-- );
  return i >= 0;
}
//---------------------------------------------------------------------------
int32_t & Exception::code(uintptr_t i) const
{
  assert( errors_.count() > 0 );
  return errors_[i].code_;
}
//---------------------------------------------------------------------------
utf8::String & Exception::what(uintptr_t i) const
{
  assert( errors_.count() > 0 );
  return errors_[i].what_;
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
  if( errors_.count() == 0 || code() == EINVAL ){
    refCount_ = 0;
    assert( 0 );
  }
  throw ExceptionSP(this);
}
//---------------------------------------------------------------------------
utf8::String Exception::stdError(utf8::String::Stream * s) const
{
  utf8::String::Stream stream;
  if( s == NULL ) s = &stream;
  for( uintptr_t i = 0; i < errors_.count(); i++ ){
    if( errors_[i].code_ == 0 ) continue;
    intmax_t a;
    utf8::String serr(strError(errors_[i].code_));
    if( !utf8::tryStr2Int(serr,a) ){
      if( errors_[i].code_ >= errorOffset ) *s << errors_[i].code_ - errorOffset; else *s << errors_[i].code_;
      *s << " ";
    }
    if( serr.strlen() > 0 ) *s << serr << " ";
    *s << errors_[i].what_ << "\n";
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

