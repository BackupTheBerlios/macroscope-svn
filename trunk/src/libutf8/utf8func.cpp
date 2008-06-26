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
#include <adicpp/ksys.h>
//---------------------------------------------------------------------------
namespace utf8 {
//---------------------------------------------------------------------------
char * strnstr(const char * s1,const char * s2,uintptr_t n)
{
  uintptr_t k;
  while( n > 0 ){
    for( k = 0;; k++ ){
      if( s2[k] == '\0' ) return (char *) s1;
      if( k >= n || s1[k] != s2[k] ) break;
    }
    s1++;
    n--;
  }
  return NULL;
}
//---------------------------------------------------------------------------
intptr_t mbcs2utf8s(uintptr_t cp, char * utf8s, uintptr_t utf8l, const char * s, uintptr_t l)
{
  const unsigned char * p, * a;
  intptr_t r = 0, ol = utf8l, low , high ;
  const utf8cp * e = findCodePage(cp);
  if( e == NULL ){
    if( cp == CP_UNICODE ){
      while( l > 0 && *(wchar_t *) s != L'\0' ){
        ucs2utf8(utf8s,utf8l,*(wchar_t *) s);
        s = (char *) ((wchar_t *) s + 1);
        l--;
      }
    }
    else while( l > 0 ){
      if( *(const unsigned char *) s == 0 ) break;
      if( (intptr_t) utf8l > 0 ){
        *(unsigned char *) utf8s = *(const unsigned char *) s;
        utf8s++;
      }
      s++;
      l--;
      utf8l--;
    }
  }
  else {
    while( l-- > 0 && *s != '\0' ){
      a = e->cps2utf8s + sizeof(uint32_t);
      low = 0;
      high = *a++ - 1;
      while( low <= high ){
        p = a + ((low + high) / 2) * 4;
        if( *(const unsigned char *) s > p[1] ){
          low = (p - a) / 4 + 1;
        }
        else if( *(const unsigned char *) s < p[0] ){
          high = (p - a) / 4 - 1;
        }
        else{
          r = ucs2utf8(*(unsigned char **) &utf8s, utf8l, *(uint16_t *) (p + 2) + *(const unsigned char *) s - *p);
          goto fd;
        }
      }
      a += a[-1] * 4;
      low = 0;
      high = *a++ - 1;
      while( low <= high ){
        p = a + ((low + high) / 2) * 3;
        if( *(const unsigned char *) s > *p ){
          low = (p - a) / 3 + 1;
        }
        else if( *(const unsigned char *) s < *p ){
          high = (p - a) / 3 - 1;
        }
        else{
          r = ucs2utf8(*(unsigned char **) &utf8s, utf8l, *(uint16_t *) (p + 1));
          goto fd;
        }
      }
      a += a[-1] * 3;
      while( *a != 0 ){
        if( *a == *(const unsigned char *) s ){
          l--; s++;
          if( l == 0 || *s == '\0' ){
            r = -3; goto fd;
          }
          low = 0;
          high = a[1] - 1;
          a += 2;
          while( low <= high ){
            p = a + ((low + high) / 2) * e->utf8s2cps[0];
            intptr_t  c = memncmp(s, p, (uintptr_t) e->utf8s2cps[0] > l ? l : e->utf8s2cps[0]);
            if( c > 0 ){
              low = (p - a) / e->utf8s2cps[0] + 1;
            }
            else if( c < 0 ){
              high = (p - a) / e->utf8s2cps[0] - 1;
            }
            else{
              r = ucs2utf8(*(unsigned char **) &utf8s, utf8l, *(uint16_t *) (p + e->utf8s2cps[0]));
              goto fd;
            }
          }
        }
        a += a[-1] * (e->utf8s2cps[0] + 2);
      }
fd:   if( r != 0 && r != -2 ) break;
      s++;
    }
  }
  if( r < 0 ){
    errno = EINVAL;
  }
  else if( intptr_t(utf8l) <= 0 ){
    r = ol - utf8l;
  }
  else {
    *utf8s = '\0';
  }
  return r;
}
//---------------------------------------------------------------------------
intptr_t utf8s2mbcs(uintptr_t cp, char * s, uintptr_t l, const char * utf8s, uintptr_t utf8l)
{
  uintptr_t c = 0;
  intptr_t r = 0, ol = l, low , high;
  const utf8cp * e = findCodePage(cp);
  if( e == NULL ){
    if( cp == CP_UNICODE ){
      uintptr_t ql;
      while( utf8l > 0 && *utf8s != '\0' ){
        c = utf82ucs(utf8s,ql);
        if( intptr_t(ql) <= 0 ){
          errno = EINVAL;
          return -1;
        }
        if( intptr_t(l) >= intptr_t(sizeof(wchar_t)) ) *(wchar_t *) s = (wchar_t) c;
        s += sizeof(wchar_t);
        utf8s += ql;
        utf8l -= ql;
        l -= sizeof(wchar_t);
      }
      if( intptr_t(l) <= 0 ){
        return ol - l;
      }
      else {
        *(wchar_t *) s = L'\0';
      }
      return 0;
    }
    while( utf8l > 0 ){
      if( *(const unsigned char *) utf8s == 0 ) break;
      if( (intptr_t) l > 0 ){
        *(unsigned char *) s = *(const unsigned char *) utf8s;
        s++;
      }
      utf8s++;
      utf8l--;
      l--;
    }
  }
  else {
    while( utf8l > 0 && *utf8s != '\0' ){
      r = utf82ucs(*(const unsigned char **) &utf8s, utf8l, c);
      if( r != 0 ) break;
      const unsigned char * p, * a  = e->utf8s2cps + 2;
      while( *a != 0xFF ){
        low = 0;
        high = *a - 1;
        a += 2;
        while( low <= high ){
          p = a + ((low + high) / 2) * (4 + a[-1]);
          if( c > (uintptr_t) *(const uint16_t *) (p + 2) ){
            low = (p - a) / (4 + a[-1]) + 1;
          }
          else if( c < (uintptr_t) *(const uint16_t *) p ){
            high = (p - a) / (4 + a[-1]) - 1;
          }
          else{
            c = (*(const uint32_t *) (p + 4) & (0xFFFFFFFFu >> (32 - a[-1] * 8))) + c - *(const uint16_t *) p;
            goto fd;
          }
        }
        a += a[-2] * (a[-1] + 4);
      }
      a += 2;
      while( *(const uint16_t *) a != 0xFFFF ){
        low = 0;
        high = *(const uint16_t *) a - 1;
        a += 3;
        while( low <= high ){
          p = a + ((low + high) / 2) * (2 + a[-1]);
          if( c > (uintptr_t) *(const uint16_t *) p ){
            low = (p - a) / (2 + a[-1]) + 1;
          }
          else if( c < (uintptr_t) *(const uint16_t *) p ){
            high = (p - a) / (2 + a[-1]) - 1;
          }
          else{
            c = *(const uint32_t *) (p + 2) & (0xFFFFFFFF >> (32 - a[-1] * 8));
            goto fd;
          }
        }
        a += *(const uint16_t *) (a - 3) * (a[-1] + 2);
      }
      if( c != 0 ){
        c = e->utf8s2cps[1];
        r = -3;
      }
fd:   if( c < 0x100 ){
        if( (intptr_t) l > 0 ) *s++ = (unsigned char) c;
        l--;
      }
      else if( c < 0x1000 ){
        if( (intptr_t) l > 1 ) *(uint16_t *) s = (uint16_t) c;
        l -= 2;
      }
      else if( c < 0x1000000 ){
        if( (intptr_t) l > 2 ){
          *(uint16_t *) s = (uint16_t) c;
          *(unsigned char *) s = (unsigned char) (c >> 24);
        }
        l -= 3;
      }
      else {
        if( (intptr_t) l > 3 ) *(uint32_t *) s = (uint32_t) c;
        l -= 4;
      }
      if( r != 0 ) break;
    }
  }
  if( r < 0 ){
    errno = EINVAL;
  }
  else if( intptr_t(l) <= 0 ){
    r = ol - l;
  }
  else {
    *s = '\0';
  }
  return r;
}
//---------------------------------------------------------------------------
uintptr_t utf8s2Lower(char * utf8sD, uintptr_t utf8lD, const char * utf8sS, uintptr_t utf8lS)
{
  intptr_t  r = 0, l = utf8lD, low , high ;

  while( utf8lS > 0 ){
    uintptr_t c = 0;
    r = utf82ucs(*(const unsigned char **) &utf8sS, utf8lS, c);
    if( r != 0 ) break;
    low = 0;
    high = (sizeof(lowerTable) / sizeof(lowerTable[0])) / 2 - 1;
    while( low <= high ){
      const uint16_t *  p = lowerTable + ((low + high) / 2) * 2;
      if( c > (uintptr_t) *p ){
        low = (p - lowerTable) / 2 + 1;
      }
      else if( c < (uintptr_t) *p ){
        high = (p - lowerTable) / 2 - 1;
      }
      else{
        c = p[1];
        break;
      }
    }
    r = ucs2utf8(*(unsigned char **) &utf8sD, utf8lD, c);
    if( c == 0 || (r != 0 && r != -2) ) break;
  }
  if( r < 0 ){
    errno = EINVAL;
  }
  else if( (intptr_t) utf8lD < 0 ) r = l - utf8lD;
  return r;
}
//---------------------------------------------------------------------------
uintptr_t utf8s2Upper(char * utf8sD, uintptr_t utf8lD, const char * utf8sS, uintptr_t utf8lS)
{
  intptr_t  r = 0, l = utf8lD, low , high ;

  while( utf8lS > 0 ){
    uintptr_t c = 0;
    r = utf82ucs(*(const unsigned char **) &utf8sS, utf8lS, c);
    if( r != 0 ) break;
    low = 0;
    high = (sizeof(upperTable) / sizeof(upperTable[0])) / 2 - 1;
    while( low <= high ){
      const uint16_t *  p = upperTable + ((low + high) / 2) * 2;
      if( c > (uintptr_t) *p ){
        low = (p - upperTable) / 2 + 1;
      }
      else if( c < (uintptr_t) *p ){
        high = (p - upperTable) / 2 - 1;
      }
      else{
        c = p[1];
        break;
      }
    }
    r = ucs2utf8(*(unsigned char **) &utf8sD, utf8lD, c);
    if( c == 0 || (r != 0 && r != -2) ) break;
  }
  if( r < 0 ){
    errno = EINVAL;
  }
  else if( (intptr_t) utf8lD < 0 ) r = l - utf8lD;
  return r;
}
//---------------------------------------------------------------------------
uintptr_t utf8strlen(const char * utf8s)
{
  uintptr_t l = 0, sl;

  assert( utf8s != NULL );
  while( *utf8s != '\0' ){
    sl = utf8seqlen((const unsigned char *) utf8s);
    if( sl == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    utf8s += sl;
    l++;
  }
  return l;
}
//---------------------------------------------------------------------------
uintptr_t utf8strlen(const char * utf8s, uintptr_t & size)
{
  uintptr_t l = 0, sl;
  const char *  s = utf8s;

  while( *s != '\0' ){
    sl = utf8seqlen((const unsigned char *) s);
    if( sl == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    s += sl;
    l++;
  }
  size = s - utf8s;
  return l;
}
//---------------------------------------------------------------------------
uintptr_t utf8strlen(const char * utf8s, uintptr_t l, uintptr_t & size)
{
  uintptr_t q = 0, sl;
  const char * s = utf8s;

  while( q < l && *s != '\0' ){
    sl = utf8seqlen((const unsigned char *) s);
    if( sl == 0 )
      newObjectV1C2<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    s += sl;
    q++;
  }
  size = s - utf8s;
  return q;
}
//---------------------------------------------------------------------------
static uint16_t lowerBuffer[65536];
//---------------------------------------------------------------------------
uintptr_t utf8c2LowerUCS(const char * utf8s)
{
  uintptr_t c = utf82ucs((const unsigned char *) utf8s);
  if( c > 32 ){
    uintptr_t k = c;
    if( lowerBuffer[k] == 0 ){
      intptr_t low = 0, high = (sizeof(lowerTable) / sizeof(lowerTable[0])) / 2 - 1;
      while( low <= high ){
        const uint16_t * p = lowerTable + ((low + high) / 2) * 2;
        if( c > (uintptr_t) *p ){
          low = (p - lowerTable) / 2 + 1;
        }
        else if( c < (uintptr_t) *p ){
          high = (p - lowerTable) / 2 - 1;
        }
        else {
          c = p[1];
          break;
        }
      }
      lowerBuffer[k] = c;
    }
    c = lowerBuffer[k];
  }
  return c;
}
//---------------------------------------------------------------------------
uintptr_t utf8c2LowerUCS(const char * utf8s, uintptr_t & l)
{
  uintptr_t c = utf82ucs((const unsigned char *) utf8s, l);
  if( c > 32 ){
    uintptr_t k = c;
    if( lowerBuffer[k] == 0 ){
      intptr_t low = 0, high = (sizeof(lowerTable) / sizeof(lowerTable[0])) / 2 - 1;
      while( low <= high ){
        const uint16_t * p = lowerTable + ((low + high) / 2) * 2;
        if( c > (uintptr_t) *p ){
          low = (p - lowerTable) / 2 + 1;
        }
        else if( c < (uintptr_t) *p ){
          high = (p - lowerTable) / 2 - 1;
        }
        else {
          c = p[1];
          break;
        }
      }
      lowerBuffer[k] = c;
    }
    c = lowerBuffer[k];
  }
  return c;
}
//---------------------------------------------------------------------------
static uint16_t upperBuffer[65536];
//---------------------------------------------------------------------------
uintptr_t utf8c2UpperUCS(const char * utf8s)
{
  uintptr_t c = utf82ucs((const unsigned char *) utf8s);
  if( c > 32 ){
    uintptr_t k = c;
    if( upperBuffer[k] == 0 ){
      intptr_t low = 0, high = (sizeof(upperTable) / sizeof(upperTable[0])) / 2 - 1;
      while( low <= high ){
        const uint16_t * p = upperTable + ((low + high) / 2) * 2;
        if( c > (uintptr_t) *p ){
          low = (p - upperTable) / 2 + 1;
        }
        else if( c < (uintptr_t) *p ){
          high = (p - upperTable) / 2 - 1;
        }
        else {
          c = p[1];
          break;
        }
      }
      upperBuffer[k] = c;
    }
    c = upperBuffer[k];
  }
  return c;
}
//---------------------------------------------------------------------------
uintptr_t utf8c2UpperUCS(const char * utf8s, uintptr_t & l)
{
  uintptr_t c = utf82ucs((const unsigned char *) utf8s, l);
  if( c > 32 ){
    uintptr_t k = c;
    if( upperBuffer[k] == 0 ){
      intptr_t low = 0, high = (sizeof(upperTable) / sizeof(upperTable[0])) / 2 - 1;
      while( low <= high ){
        const uint16_t * p = upperTable + ((low + high) / 2) * 2;
        if( c > (uintptr_t) *p ){
          low = (p - upperTable) / 2 + 1;
        }
        else if( c < (uintptr_t) *p ){
          high = (p - upperTable) / 2 - 1;
        }
        else {
          c = p[1];
          break;
        }
      }
      upperBuffer[k] = c;
    }
    c = upperBuffer[k];
  }
  return c;
}
//---------------------------------------------------------------------------
} // namespace utf8
//---------------------------------------------------------------------------
