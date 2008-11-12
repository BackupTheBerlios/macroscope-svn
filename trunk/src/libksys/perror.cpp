/*-
 * Copyright 2005-2008 Guram Dukashvili
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
//#include <adicpp/pwerrs.h>
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
    for( i = strErrorHandlers().count() - 1; i >= 0; i-- ){
      utf8::String se(strErrorHandlers()[i](err));
      if( !se.isNull() > 0 ) return se;
    }
#if defined(__WIN32__) || defined(__WIN64__)
    if( err >= errorOffset ){
      DWORD bytes;
      LPVOID lpMsgBuf = NULL;
      utf8::String wes;
      try {
        if( isWin9x() ){
          bytes = FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err - errorOffset,
            LANG_USER_DEFAULT, // Default language
            (LPSTR) &lpMsgBuf,
            0,
            NULL 
          );
          if( bytes > 0 ){
            ((LPSTR) lpMsgBuf)[bytes - 1] = '\0';
            LPSTR nl = strchr((LPSTR) lpMsgBuf,'\r');
            if( nl != NULL ) *nl = '\0';
            nl = strchr((LPSTR) lpMsgBuf,'\n');
            if( nl != NULL ) *nl = '\0';
            wes = (LPCSTR) lpMsgBuf;
          }
        }
        else {
          bytes = FormatMessageW(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            err - errorOffset,
            LANG_USER_DEFAULT, // Default language
            (LPWSTR) &lpMsgBuf,
            0,
            NULL 
          );
          if( bytes > 0 ){
            ((LPWSTR) lpMsgBuf)[bytes - 1] = L'\0';
            LPWSTR nl = wcschr((LPWSTR) lpMsgBuf,L'\r');
            if( nl != NULL ) *nl = L'\0';
            nl = wcschr((LPWSTR) lpMsgBuf,L'\n');
            if( nl != NULL ) *nl = L'\0';
            wes = (LPCWSTR) lpMsgBuf;
          }
        }
      }
      catch( ... ){
        LocalFree(lpMsgBuf);
        throw;
      }
      LocalFree(lpMsgBuf);
      if( bytes > 0 ) return wes;
    }
/*    intptr_t c = sizeof(winErrsDesc) / sizeof(winErrsDesc[0]) - 1;
    if( err >= errorOffset && err < winErrsDesc[c].code_ + errorOffset ){
      struct WinErrDesc bs;
      bs.code_ = err - errorOffset;
      c = sizeof(winErrsDesc) / sizeof(winErrsDesc[0]);
      i = bSearch<WinErrDesc>(winErrsDesc, bs, c);
      if( c == 0 ) return winErrsDesc[i].text_;
      err -= errorOffset;
    }*/
#endif
    AutoPtr<char> serr;
#if HAVE_STRERROR_R
    int32_t er;
    size_t sel = 16;
    for(;;){
#if (defined(_GNU_SOURCE) || defined(__USE_GNU)) && !__freebsd__
      char * p = strerror_r(err,serr,sel);
      sel = strlen(p) + 1;
      serr.reallocT(sel);
      strcpy(serr,p);
      er = 0;
#else
      serr.reallocT(sel = (sel << 1) + (sel == 0));
      er = strerror_r(err,serr,sel);
#endif
      if( er == 0 || er == EINVAL ) break;
      if( er != ERANGE ){
        perror(NULL);
	abort();
      }
    }
#elif HAVE_STRERROR
    static ilock_t mutex = 0;
    AutoInterlockedLock<ilock_t> lock(mutex);
    char * p = strerror(err);
    serr.reallocT(::strlen(p) + 1);
    strcpy(serr,p);
#endif
    if( strchr(serr.ptr(),'\r') != NULL ) *strchr(serr.ptr(),'\r') = '\0';
    if( strchr(serr.ptr(),'\n') != NULL ) *strchr(serr.ptr(),'\n') = '\0';
    if( strlen(serr) > 0 && ::strncmp(serr,"Unknown error",13) != 0 ) return serr.ptr();
    return utf8::int2Str(err);
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
void strErrorInitialize()
{
  new (strErrorHandlersHolder) Array<StrErrorHandler>;
#if defined(__WIN32__) || defined(__WIN64__)
//  qSort(winErrsDesc, 0, sizeof(winErrsDesc) / sizeof(winErrsDesc[0]) - 1);
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

