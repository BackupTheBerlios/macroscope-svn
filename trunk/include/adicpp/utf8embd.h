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
#ifndef _utf8embdH_
#define _utf8embdH_
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
#define CP_ACP        0
#define CP_OEMCP      1
#define CP_MACCP      2
#define CP_THREAD_ACP 3
#define CP_SYMBOL     42
#ifndef CP_UNICODE
#define CP_UNICODE    1200
#endif
#define CP_UTF7       65000
#define CP_UTF8       65001
#define CP_UNIXCP     65010
//---------------------------------------------------------------------------
#ifndef C1_UPPER
#define C1_UPPER   0x0001
#define C1_LOWER   0x0002
#define C1_DIGIT   0x0004
#define C1_SPACE   0x0008
#define C1_PUNCT   0x0010
#define C1_CNTRL   0x0020
#define C1_BLANK   0x0040
#define C1_XDIGIT  0x0080
#define C1_ALPHA   0x0100
#define C1_DEFINED 0x0200
#endif

#ifndef C2_LEFTTORIGHT
#define C2_LEFTTORIGHT      0x0001
#define C2_RIGHTTOLEFT      0x0002
#define C2_EUROPENUMBER     0x0003
#define C2_EUROPESEPARATOR  0x0004
#define C2_EUROPETERMINATOR 0x0005
#define C2_ARABICNUMBER     0x0006
#define C2_COMMONSEPARATOR  0x0007
#define C2_BLOCKSEPARATOR   0x0008
#define C2_SEGMENTSEPARATOR 0x0009
#define C2_WHITESPACE       0x000A
#define C2_OTHERNEUTRAL     0x000B
#define C2_NOTAPPLICABLE    0x0000
#endif

#ifndef C3_NONSPACING
#define C3_NONSPACING    0x0001
#define C3_DIACRITIC     0x0002
#define C3_VOWELMARK     0x0004
#define C3_SYMBOL        0x0008
#define C3_KATAKANA      0x0010
#define C3_HIRAGANA      0x0020
#define C3_HALFWIDTH     0x0040
#define C3_FULLWIDTH     0x0080
#define C3_IDEOGRAPH     0x0100
#define C3_KASHIDA       0x0200
#define C3_LEXICAL       0x0400
#define C3_ALPHA         0x8000
#define C3_NOTAPPLICABLE 0x0000
#endif
//---------------------------------------------------------------------------
extern const uint32_t C1Table[];
extern const uint32_t C2Table[];
extern const uint32_t C3Table[];
//---------------------------------------------------------------------------
inline uintptr_t imul11(uintptr_t c)
{
  return c * 8u + c * 2u + c;
}
//---------------------------------------------------------------------------
inline uintptr_t getC1Type(uintptr_t c)
{
  return *(uint16_t *) ((uint8_t *) C1Table + c * 9u / 8u) >> (c * 9u % 8u);
}
//---------------------------------------------------------------------------
inline uintptr_t getC2Type(uintptr_t c)
{
  return *((uint8_t *) C2Table + c * 4u / 8u) >> (c * 4u % 8u);
}
//---------------------------------------------------------------------------
inline uintptr_t getC3Type(uintptr_t c)
{
  return *(uint16_t *) ((uint8_t *) C3Table + imul11(c) / 8u) >> (imul11(c) % 8u);
}
//---------------------------------------------------------------------------
struct utf8cp {
  const uint8_t * cps2utf8s;
  const uint8_t * utf8s2cps;
};
//---------------------------------------------------------------------------
extern const utf8cp   utf8cps[];
extern const uintptr_t utf8cpsCount;
//---------------------------------------------------------------------------
extern uintptr_t       cpansi;
extern uintptr_t       cpoem;
extern const utf8cp * cpansip;
extern const utf8cp * cpoemp;
//---------------------------------------------------------------------------
const utf8cp *  findCodePage(uintptr_t cp);
uintptr_t       detectCodePages();
uintptr_t       getCodePage(uintptr_t cp);
//---------------------------------------------------------------------------
intptr_t        mbcs2utf8s(uintptr_t cp, char * utf8s, uintptr_t utf8l, const char * s, uintptr_t l);
intptr_t        utf8s2mbcs(uintptr_t cp, char * s, uintptr_t l, const char * utf8s, uintptr_t utf8l);
//---------------------------------------------------------------------------
extern const uint16_t lowerTable[1346];
extern const uint16_t upperTable[1272];
//---------------------------------------------------------------------------
uintptr_t       utf8c2LowerUCS(const char * utf8s);
uintptr_t       utf8c2LowerUCS(const char * utf8s, uintptr_t & l);
uintptr_t       utf8c2UpperUCS(const char * utf8s);
uintptr_t       utf8c2UpperUCS(const char * utf8s, uintptr_t & l);
uintptr_t       utf8s2Upper(char * utf8sD, uintptr_t utf8lD, const char * utf8sS, uintptr_t utf8lS);
uintptr_t       utf8s2Lower(char * utf8sD, uintptr_t utf8lD, const char * utf8sS, uintptr_t utf8lS);
uintptr_t       utf8s2Upper(char * utf8sD, uintptr_t utf8lD, const char * utf8sS, uintptr_t utf8lS);
//---------------------------------------------------------------------------
inline uintptr_t utf82ucs(const unsigned char * utf8s)
{
  uintptr_t c;

#if SIZEOF_WCHAR_t_T > 2
  if( (*utf8s & 0xFC) == 0xFC && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 && utf8s[4] != 0 && utf8s[5] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x1) << 30) | ((uintptr_t) (utf8s[1] & 0x3F) << 24) | ((uintptr_t) (utf8s[2] & 0x3F) << 18) | ((uintptr_t) (utf8s[3] & 0x3F) << 12) | ((uintptr_t) (utf8s[4] & 0x3F) << 6) | (uintptr_t) (utf8s[5] & 0x3F);
  }
  else if( (*utf8s & 0xF8) == 0xF8 && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 && utf8s[4] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x3) << 24) | ((uintptr_t) (utf8s[1] & 0x3F) << 18) | ((uintptr_t) (utf8s[2] & 0x3F) << 12) | ((uintptr_t) (utf8s[3] & 0x3F) << 6) | (uintptr_t) (utf8s[4] & 0x3F);
  }
  else if( (*utf8s & 0xF0) == 0xF0 && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x7) << 18) | ((uintptr_t) (utf8s[1] & 0x3F) << 12) | ((uintptr_t) (utf8s[2] & 0x3F) << 6) | (uintptr_t) (utf8s[3] & 0x3F);
  }
  else
    #endif
  if( (*utf8s & 0xE0) == 0xE0 && utf8s[1] != 0 && utf8s[2] != 0 ){
    c = ((uintptr_t) (*utf8s & 0xF) << 12) | ((uintptr_t) (utf8s[1] & 0x3F) << 6) | (uintptr_t) (utf8s[2] & 0x3F);
  }
  else if( (*utf8s & 0xC0) == 0xC0 && utf8s[1] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x1F) << 6) | (uintptr_t) (utf8s[1] & 0x3F);
  }
  else if( *utf8s < 0x80 ){
    c = *utf8s;
  }
  else{
    c = uintptr_t(intptr_t(-1));
  }
  return c;
}
//---------------------------------------------------------------------------
inline uintptr_t utf82ucs(const char * utf8s)
{
  return utf82ucs(reinterpret_cast< const unsigned char *>(utf8s));
}
//---------------------------------------------------------------------------
inline uintptr_t utf82ucs(const unsigned char * utf8s, uintptr_t & l)
{
  uintptr_t c;

  l = intptr_t(-1);
#if SIZEOF_WCHAR_t_T > 2
  if( (*utf8s & 0xFC) == 0xFC && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 && utf8s[4] != 0 && utf8s[5] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x1) << 30) | ((uintptr_t) (utf8s[1] & 0x3F) << 24) | ((uintptr_t) (utf8s[2] & 0x3F) << 18) | ((uintptr_t) (utf8s[3] & 0x3F) << 12) | ((uintptr_t) (utf8s[4] & 0x3F) << 6) | (uintptr_t) (utf8s[5] & 0x3F);
    l = 6;
  }
  else if( (*utf8s & 0xF8) == 0xF8 && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 && utf8s[4] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x3) << 24) | ((uintptr_t) (utf8s[1] & 0x3F) << 18) | ((uintptr_t) (utf8s[2] & 0x3F) << 12) | ((uintptr_t) (utf8s[3] & 0x3F) << 6) | (uintptr_t) (utf8s[4] & 0x3F);
    l = 5;
  }
  else if( (*utf8s & 0xF0) == 0xF0 && utf8s[1] != 0 && utf8s[2] != 0 && utf8s[3] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x7) << 18) | ((uintptr_t) (utf8s[1] & 0x3F) << 12) | ((uintptr_t) (utf8s[2] & 0x3F) << 6) | (uintptr_t) (utf8s[3] & 0x3F);
    l = 4;
  }
  else
#endif
  if( (*utf8s & 0xE0) == 0xE0 && utf8s[1] != 0 && utf8s[2] != 0 ){
    c = ((uintptr_t) (*utf8s & 0xF) << 12) | ((uintptr_t) (utf8s[1] & 0x3F) << 6) | (uintptr_t) (utf8s[2] & 0x3F);
    l = 3;
  }
  else if( (*utf8s & 0xC0) == 0xC0 && utf8s[1] != 0 ){
    c = ((uintptr_t) (*utf8s & 0x1F) << 6) | (uintptr_t) (utf8s[1] & 0x3F);
    l = 2;
  }
  else if( *utf8s < 0x80 ){
    c = *utf8s;
    l = 1;
  }
  else {
    c = uintptr_t(intptr_t(-1));
    l = 1;
  }
  return c;
}
//---------------------------------------------------------------------------
inline uintptr_t utf82ucs(const char * utf8s, uintptr_t & l)
{
  return utf82ucs(reinterpret_cast< const unsigned char *>(utf8s), l);
}
//---------------------------------------------------------------------------
inline uintptr_t utf8seqlen(const unsigned char * utf8s)
{
  uintptr_t l;

#if SIZEOF_WCHAR_T > 2
  if( (*utf8s & 0xFC) == 0xFC ){
    l = 6;
  }
  else if( (*utf8s & 0xF8) == 0xF8 ){
    l = 5;
  }
  else if( (*utf8s & 0xF0) == 0xF0 ){
    l = 4;
  }
  else
#endif
  if( (*utf8s & 0xE0) == 0xE0 ){
    l = 3;
  }
  else if( (*utf8s & 0xC0) == 0xC0 ){
    l = 2;
  }
  else if( *utf8s < 0x80 ){
    l = 1;
  }
  else{
    l = 0;
  }
  return l;
}
//---------------------------------------------------------------------------
inline uintptr_t utf8seqlen(const char * utf8s)
{
  return utf8seqlen(reinterpret_cast< const unsigned char *>(utf8s));
}
//---------------------------------------------------------------------------
uintptr_t utf8strlen(const char * utf8s);
uintptr_t utf8strlen(const char * utf8s, uintptr_t & size);
uintptr_t utf8strlen(const char * utf8s, uintptr_t l, uintptr_t & size);
//---------------------------------------------------------------------------
inline uintptr_t ucs2utf8seq(unsigned char * utf8s, uintptr_t c)
{
  uintptr_t utf8l;

  if( c < 0x80 ){
    *utf8s++ = (unsigned char) c;
    utf8l = 1;
  }
  else if( c < 0x800 ){
    utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[0] = (unsigned char) (0xC0 | c);
    utf8l = 2;
  }
  else if( c < 0x10000 ){
    utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[0] = (unsigned char) (0xE0 | c);
    utf8l = 3;
  }
#if SIZEOF_WCHAR_T_T > 2
  else if( c < 0x200000 ){
    utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[0] = (unsigned char) (0xF0 | c);
    utf8l = 4;
  }
  else if( c < 0x4000000 ){
    utf8s[4] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[0] = (unsigned char) (0xF8 | c);
    utf8l = 5;
  }
  else if( c < 0x8000000 ){
    utf8s[5] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[4] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
    c >>= 6;
    utf8s[0] = (unsigned char) (0xFC | c);
    utf8l = 6;
  }
#endif
  else{
    utf8l = 0;
  }
  return utf8l;
}
//---------------------------------------------------------------------------
inline uintptr_t ucs2utf8seq(char * utf8s, uintptr_t c)
{
  return ucs2utf8seq((unsigned char *) utf8s, c);
}
//---------------------------------------------------------------------------
inline uintptr_t ucs2utf8seq(unsigned char * utf8s, uintptr_t utf8l, uintptr_t c)
{
  if( c < 0x80 ){
    if( utf8l > 0 )
      *utf8s++ = (char) c;
    utf8l = 1;
  }
  else if( c < 0x800 ){
    if( utf8l > 1 ){
      utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[0] = (unsigned char) (0xC0 | c);
    }
    utf8l = 2;
  }
  else if( c < 0x10000 ){
    if( utf8l > 2 ){
      utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[0] = (unsigned char) (0xE0 | c);
    }
    utf8l = 3;
  }
#if SIZEOF_WCHAR_T > 2
  else if( c < 0x200000 ){
    if( utf8l > 3 ){
      utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[0] = (unsigned char) (0xF0 | c);
    }
    utf8l = 4;
  }
  else if( c < 0x4000000 ){
    if( utf8l > 4 ){
      utf8s[4] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[0] = (unsigned char) (0xF8 | c);
    }
    utf8l = 5;
  }
  else if( c < 0x8000000 ){
    if( utf8l > 5 ){
      utf8s[5] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[4] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[3] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[2] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[1] = (unsigned char) (0x80 | (c & 0x3F));
      c >>= 6;
      utf8s[0] = (unsigned char) (0xFC | c);
    }
    utf8l = 6;
  }
#endif
  else{
    utf8l = 0;
  }
  return utf8l;
}
//---------------------------------------------------------------------------
inline uintptr_t ucs2utf8seq(char * utf8s, uintptr_t utf8l, uintptr_t c)
{
  return ucs2utf8seq((unsigned char *) utf8s, utf8l, c);
}
//---------------------------------------------------------------------------
intptr_t  utf82ucs(const unsigned char *& utf8s, uintptr_t & utf8l, uintptr_t & c);
//---------------------------------------------------------------------------
inline intptr_t utf82ucs(const char *& utf8s, uintptr_t & utf8l, uintptr_t & c)
{
  return utf82ucs(*(const unsigned char **) &utf8s, utf8l, c);
}
//---------------------------------------------------------------------------
intptr_t  ucs2utf8(unsigned char *& utf8s, uintptr_t & utf8l, uintptr_t c);
//---------------------------------------------------------------------------
inline intptr_t ucs2utf8(char *& utf8s, uintptr_t & utf8l, uintptr_t c)
{
  return ucs2utf8(*(unsigned char **) &utf8s, utf8l, c);
}
//---------------------------------------------------------------------------
intptr_t memncmp(const void * m1, const void * m2, uintptr_t n);
char * strnstr(const char * s1, const char * s2,uintptr_t n);
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
