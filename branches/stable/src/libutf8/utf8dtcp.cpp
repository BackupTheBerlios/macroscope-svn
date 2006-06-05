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
#include <adicpp/lconfig.h>
#include <adicpp/utf8embd.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
uintptr_t cpansi = 0;
uintptr_t cpoem = 0;
const utf8cp * cpansip = NULL;
const utf8cp * cpoemp = NULL;
//---------------------------------------------------------------------------
static const struct CharsetEntry {
  const char * charsetName_;
  uint32_t codepage_;
} charsetNames[] = {
  { "BIG5", 950 },
  { "CP1250", 1250 },
  { "CP1251", 1251 },
  { "CP1252", 1252 },
  { "CP1253", 1253 },
  { "CP1254", 1254 },
  { "CP1255", 1255 },
  { "CP1256", 1256 },
  { "CP1257", 1257 },
  { "CP1258", 1258 },
  { "CP932", 932 },
  { "CP936", 936 },
  { "CP949", 949 },
  { "CP950", 950 },
  { "EUCJP", 20932 },
  { "GB2312", 936 },
  { "IBM037", 37 },
  { "IBM1026", 1026 },
  { "IBM424", 424 },
  { "IBM437", 437 },
  { "IBM500", 500 },
  { "IBM850", 850 },
  { "IBM852", 852 },
  { "IBM855", 855 },
  { "IBM857", 857 },
  { "IBM860", 860 },
  { "IBM861", 861 },
  { "IBM862", 862 },
  { "IBM863", 863 },
  { "IBM864", 864 },
  { "IBM865", 865 },
  { "IBM866", 866 },
  { "IBM869", 869 },
  { "IBM874", 874 },
  { "IBM875", 875 },
  { "ISO88591", 28591 },
  { "ISO885910", 28600 },
  { "ISO885913", 28603 },
  { "ISO885914", 28604 },
  { "ISO885915", 28605 },
  { "ISO88592", 28592 },
  { "ISO88593", 28593 },
  { "ISO88594", 28594 },
  { "ISO88595", 28595 },
  { "ISO88596", 28596 },
  { "ISO88597", 28597 },
  { "ISO88598", 28598 },
  { "ISO88599", 28599 },
  { "UTF8", CP_UTF8 },
  { "KOI8-R", 20866 },
  { "KOI8-U", 20866 }
};
//---------------------------------------------------------------------------
static
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t mapCharsetNameToCodepage(const char * charset)
{
  const struct CharsetEntry * bsa = charsetNames;
  intptr_t low = 0, high = sizeof(charsetNames) / sizeof(charsetNames[0]) - 1;
  intptr_t pos, c;

  while( low <= high ){
    pos = (low + high) / 2;
#if HAVE_STRCASECMP
    c = strcasecmp(charset,bsa[pos].charsetName_);
#elif HAVE__STRICMP
    c = _stricmp(charset,bsa[pos].charsetName_);
#elif HAVE_STRICMP
    c = stricmp(charset,bsa[pos].charsetName_);
#else
    c = strcmp(charset,bsa[pos].charsetName_);
#endif
    if( c > 0 ){
      low = pos + 1;
    }
    else if( c < 0 ){
      high = pos - 1;
    }
    else return bsa[pos].codepage_;
  }
  return CP_UTF8;
}
//---------------------------------------------------------------------------
static
#ifndef __BCPLUSPLUS__
inline
#endif
uintptr_t getANSICodepage()
{
#if defined(__WIN32__) || defined(__WIN64__)
  return GetACP();
#else
  uintptr_t cp = CP_UTF8;
  if( cpansi == 0 ){
    char * lang;
    if ( (lang = getenv( "LC_ALL" )) ||
         (lang = getenv( "LANGUAGE" )) ||
         (lang = getenv( "LANG" )) ){
      char * buf = (char *) malloc(strlen(lang) + 1);
      strcpy(buf,lang);
      lang = buf;
      do {
        char * next, * dialect, * charset, * country;
        if( (next = strchr(lang,':')) != NULL ) *next++ = '\0';
        if( (dialect = strchr(lang,'@')) != NULL ) *dialect++ = '\0';
        if( (charset = strchr(lang,'.')) != NULL ) *charset++ = '\0';
        if( (country = strchr(lang,'_')) != NULL ) *country++ = '\0';
        if( (cp = mapCharsetNameToCodepage(charset)) == CP_UTF8 ) break;
        lang = next;
      } while( lang != NULL );
      free(lang);
    }
    cpansi = cp;
  }
  else {
    cp = cpansi;
  }
  return cp;
#endif
}
//---------------------------------------------------------------------------
static inline uintptr_t getOEMCodepage()
{
#if defined(__WIN32__) || defined(__WIN64__)
  return GetOEMCP();
#else
  return getANSICodepage();
#endif
}
//---------------------------------------------------------------------------
const utf8cp * findCodePage(uintptr_t cp)
{
  if( cp == CP_ACP ){
    cp = cpansi;
  }
  else if( cp == CP_OEMCP ){
    cp = cpoem;
  }
  if( cp != CP_UTF8 ){
    intptr_t low = 0, high = utf8cpsCount - 1;
    while( low <= high ){
      const utf8cp * e = utf8cps + (low + high) / 2;
      if( cp > (uint32_t) *(uint32_t *) e->cps2utf8s ){
        low = e - utf8cps + 1;
      }
      else if( cp < (uint32_t) *(uint32_t *) e->cps2utf8s ){
        high = e - utf8cps - 1;
      }
      else
        return e;
    }
  }
  return NULL;
}
//---------------------------------------------------------------------------
uintptr_t detectCodePages()
{
  cpansip = findCodePage(cpansi = getANSICodepage());
  cpoemp = findCodePage(cpoem = getOEMCodepage());
  return cpansi;
}
//---------------------------------------------------------------------------
uintptr_t getCodePage(uintptr_t cp)
{
  if( cpansi == 0 || cpoem == 0 ) detectCodePages();
  if( cp == CP_ACP ){
    cp = cpansi;
  }
  else if( cp == CP_OEMCP ){
    cp = cpoem;
  }
  return cp;
}
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------

