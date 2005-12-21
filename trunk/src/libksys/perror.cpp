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
#include <adicpp/pwerrs.h>
//---------------------------------------------------------------------------
namespace ksys {
//---------------------------------------------------------------------------
uint8_t strErrorHandlersHolder[sizeof(StrErrorHandlers)];
//---------------------------------------------------------------------------
void addStrErrorHandler(StrErrorHandler strErrorHandler)
{
  strErrorHandlers().add(strErrorHandler);
}
//---------------------------------------------------------------------------
utf8::String strError(int32_t err)
{
  if( err - errorOffset != 0 ){
    intptr_t i;
    for( i = strErrorHandlers().count() - 1; i >= 0; i-- )
      if( strErrorHandlers()[i](err).strlen() > 0 )
        return strErrorHandlers()[i](err);
#if defined(__WIN32__) || defined(__WIN64__)
    intptr_t c = sizeof(winErrsDesc) / sizeof(winErrsDesc[0]) - 1;
    if( err >= errorOffset && err < winErrsDesc[c].code_ + errorOffset ){
      struct WinErrDesc bs;
      bs.code_ = err - errorOffset;
      c = sizeof(winErrsDesc) / sizeof(winErrsDesc[0]);
      i = bSearch<WinErrDesc>(winErrsDesc,bs,c);
      if( c == 0 ) return utf8::string(winErrsDesc[i].text_);
      err -= errorOffset;
    }
#endif
    int32_t er = 0;
    AutoPtr<char> serr;
#if HAVE_STRERROR_R
    size_t serrs = 1;
    for( 
      serr.realloc(serrs);
      strerror_r(err,serr,serrs) != 0 && (er = errno) == ERANGE;
      serr.realloc(serrs <<= 1),
      memset(serr,0,serrs)
    );
#endif
#if HAVE_STRERROR
    if( er != 0 || strlen(serr) == 0 ){
      serr.realloc(strlen(strerror(err)) + 1);
      strcpy(serr,strerror(err));
      er = 0;
    }
#endif
    if( er != 0 )
      throw ExceptionSP(new Exception(er,utf8::string(__PRETTY_FUNCTION__)));
    if( strchr(serr,'\r') != NULL ) *strchr(serr,'\r') = '\0';
    if( strchr(serr,'\n') != NULL ) *strchr(serr,'\n') = '\0';
    if( strlen(serr) > 0 && strcmp(serr,"Unknown error") != 0 ) return utf8::string(serr);
    return utf8::int2Str(err);
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
void strErrorInitialize()
{
  new (strErrorHandlersHolder) Array<StrErrorHandler>;
#if defined(__WIN32__) || defined(__WIN64__)
  qSort(winErrsDesc,0,sizeof(winErrsDesc) / sizeof(winErrsDesc[0]) - 1);
#endif
}
//---------------------------------------------------------------------------
void strErrorCleanup()
{
  strErrorHandlers().~StrErrorHandlers();
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

