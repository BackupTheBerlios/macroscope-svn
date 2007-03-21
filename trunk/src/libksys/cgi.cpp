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
CGI::CGI() : method_(cgiNone)
{
}
//------------------------------------------------------------------------------
void CGI::initialize()
{
  switch( method() ){
    case cgiNone :
      break;
    case cgiPOST :
      initalizeByMethodPOST();
    case cgiGET  :
      initalizeByMethodGET();
      print(utf8::String::print("Content-Type:text/html;charset=utf8%c%c\n",13,10));
    case cgiHEAD  :
      ;
  }
}
//------------------------------------------------------------------------------
CGI & CGI::print(const utf8::String & s)
{
  if( fwrite(s.c_str(),s.size(),1,stdout) != 1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
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
    if( name.strlen() > 0 ){
    }
    i = k;
  }
}
//------------------------------------------------------------------------------
void CGI::initalizeByMethodPOST()
{
  AutoPtr<char> b;
  uintptr_t count(utf8::str2Int(getEnv("CONTENT_LENGTH")));
  b.alloc(count + 1);
  if( fread(b,count,1,stdin) != 1 ){
    int32_t err = errno;
    newObjectV1C2<Exception>(err,__PRETTY_FUNCTION__)->throwSP();
  }
  b[count] = '\0';
  char * src = b, * last = src + count - 1, * dest = src;
  while( src < last ){
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
  count = dest - src;
  b[count] = '\0';
  utf8::String::Container * container = newObjectV1V2<utf8::String::Container>(0,b.ptr());
  b.ptr(NULL);
  queryString_ = container;
  initalizeByMethodGET();
}
//------------------------------------------------------------------------------
CGIMethod CGI::method()
{
  if( method_ == cgiNone ){
    if( getEnv("GATEWAY_INTERFACE").strlen() > 0 ){
      utf8::String requestMethod(getEnv("REQUEST_METHOD"));
      if( requestMethod.strcasecmp("POST") == 0 ) method_ = cgiPOST;
      else
      if( requestMethod.strcasecmp("GET") == 0 ) method_ = cgiGET;
      else
      if( requestMethod.strcasecmp("HEAD") == 0 ) method_ = cgiHEAD;
      else
        method_ = ((queryString_ = getEnv("QUERY_STRING")).strlen() > 0 ? cgiGET : cgiPOST);
    }
  }
  return method_;
}
//------------------------------------------------------------------------------
} // namespace ksys
//------------------------------------------------------------------------------
