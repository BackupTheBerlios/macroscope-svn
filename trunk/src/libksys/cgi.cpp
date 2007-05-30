/*-
 * Copyright 2007 Guram Dukashvili
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
//------------------------------------------------------------------------------
namespace ksys {
//------------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
CGI::~CGI()
{
}
//------------------------------------------------------------------------------
CGI::CGI() : method_(cgiInit)
{
  paramsHash_.param() = &params_;
  out_.fileName("stdout").open();
}
//------------------------------------------------------------------------------
void CGI::initialize()
{
  switch( method() ){
    case cgiInit :
    case cgiNone :
      break;
    case cgiPOST :
      initalizeByMethodPOST();
    case cgiGET  :
      initalizeByMethodGET();
      out_ << utf8::String::print("Content-Type:text/html;charset=utf-8%c%c\n",13,10);
    case cgiHEAD  :
      ;
  }
}
//------------------------------------------------------------------------------
utf8::String CGI::uudecode(const utf8::String & string)
{
  uintptr_t count = string.size();
  if( count > 0 ){
    AutoPtr<char> b;
    b.alloc(count + 1);
    const char * src = string.c_str(), * last = src + count;
    char * dest = b;
    for( count = 0; src < last; count++ ){
      if( *src == '+' ){
        *dest++ = ' ';
        src++;
      }
      else if( *src == '%' ){
        int code;
        if( sscanf(src + 1,"%2x",&code) != 1 ) code = '?';
        *dest++ = (char) code;
        src += 3;
      }
      else {
        *dest++ = *src++;
      }
    }
    b[count] = '\0';
    b.realloc(count + 1);
    utf8::String::Container * container = newObjectV1V2<utf8::String::Container>(0,b.ptr());
    b.ptr(NULL);
    return container;
  }
  return utf8::String();
}
//------------------------------------------------------------------------------
void CGI::initalizeByMethodGET()
{
  utf8::String::Iterator i(queryString_);
  while( !i.eof() ){
    utf8::String::Iterator j(i);
    while( !i.eof() && i.getChar() != '=' ) i.next();
    utf8::String::Iterator k(i + 1);
    while( !k.eof() && k.getChar() != '&' ) k.next();
    utf8::String name(j,i);
    utf8::String value(i + 1,k);
    if( name.trim().strlen() == 0 )
      newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    paramsHash_.insert(params_[params_.add(Param(uudecode(name),uudecode(value))).count() - 1]);
    i = k + 1;
  }
}
//------------------------------------------------------------------------------
void CGI::initalizeByMethodPOST()
{
  uintptr_t count((uintptr_t) utf8::str2Int(getEnv("CONTENT_LENGTH")));
  if( count > 0 ){
    AutoPtr<char> b;
    b.alloc(count + 1);
    if( fread(b,count,1,stdin) != 1 && count > 0 ){
      int32_t err = errno;
      newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
    }
    b[count] = '\0';
    utf8::String::Container * container = newObjectV1V2<utf8::String::Container>(0,b.ptr());
    b.ptr(NULL);
    queryString_ = container;
  }
  else {
    queryString_ = utf8::String();
  }
}
//------------------------------------------------------------------------------
CGIMethod CGI::method() const
{
  if( method_ == cgiInit ){
    if( getEnv("GATEWAY_INTERFACE").strlen() > 0 ){
      utf8::String requestMethod(getEnv("REQUEST_METHOD"));
      queryString_ = getEnv("QUERY_STRING");
      if( requestMethod.strcasecmp("POST") == 0 ){
        method_ = cgiPOST;
      }
      else if( requestMethod.strcasecmp("GET") == 0 ){
        method_ = cgiGET;
      }
      else if( requestMethod.strcasecmp("HEAD") == 0 ){
        method_ = cgiHEAD;
      }
      else if( queryString_.trim().strlen() > 0 ){
        method_ = cgiGET;
      }
      else
        method_ = cgiPOST;
    }
    else {
      method_ = cgiNone;
    }
  }
  return method_;
}
//------------------------------------------------------------------------------
utf8::String CGI::paramAsString(const utf8::String & name,const utf8::String & defValue)
{
  intptr_t i = paramIndex(name);
  return i < 0 ? defValue : params_[i].value_;
}
//------------------------------------------------------------------------------
utf8::String CGI::paramAsString(uintptr_t i,const utf8::String & defValue)
{
  if( i >= params_.count() )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return params_[i].value_;
}
//------------------------------------------------------------------------------
Mutant CGI::paramAsMutant(const utf8::String & name,const Mutant & defValue)
{
  intptr_t i = paramIndex(name);
  return i < 0 ? defValue : Mutant(params_[i].value_);
}
//------------------------------------------------------------------------------
Mutant CGI::paramAsMutant(uintptr_t i,const Mutant & defValue)
{
  if( i >= params_.count() )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return params_[i].value_;
}
//------------------------------------------------------------------------------
intptr_t CGI::paramIndex(const utf8::String & name,bool noThrow)
{
  Param t(name);
  Param * param = paramsHash_.find(t);
  if( param == NULL ){
    if( !noThrow ) newObjectV1C2<Exception>(ENOENT,__PRETTY_FUNCTION__)->throwSP();
    return -1;
  }
  return param - &params_[0];
}
//------------------------------------------------------------------------------
utf8::String CGI::paramName(uintptr_t i)
{
  if( i >= params_.count() )
    newObjectV1C2<Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  return params_[i].name_;
}
//------------------------------------------------------------------------------
uintptr_t CGI::paramCount()
{
  return params_.count();
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
