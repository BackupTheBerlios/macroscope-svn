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
#ifdef __BCPLUSPLUS__
using namespace utf8;
#endif
//---------------------------------------------------------------------------
namespace ksys {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Mutant & Mutant::clear()
{
  switch( type_ ){
    case      mtNull :
    case       mtInt :
    case     mtFloat :
    case      mtTime :
    case      mtCStr :
    case      mtWStr :
    case       mtStr :
      break;
    case    mtString :
      string().~String();
      break;
    case    mtBinary :
      stream().~MemoryStream();
      break;
  }
  type_ = mtNull;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
void Mutant::initialize()
{
}
//---------------------------------------------------------------------------
void Mutant::cleanup()
{
}
//---------------------------------------------------------------------------
bool Mutant::isBooleanString(const utf8::String & string, bool & value)
{
  bool  r, a1, a2;
  a1 =
    string.strcasecmp("TRUE") == 0 || string.strcasecmp("YES") == 0 ||
    string.strcasecmp("ON") == 0 || string.strcasecmp("ACCEPT") == 0 ||
    string.strcasecmp("PASS") == 0 || string.strcasecmp("PERMIT") == 0 ||
    string.strcasecmp("ALLOW") == 0 || string.strcasecmp("ALLOWED") == 0 ||
    string.strcasecmp("ENABLE") == 0 || string.strcasecmp("ENABLED") == 0
  ;
  a2 =
      string.strcasecmp("FALSE") == 0 || string.strcasecmp("NO") == 0 ||
      string.strcasecmp("OFF") == 0 || string.strcasecmp("DENY") == 0 ||
      string.strcasecmp("DENIED") == 0 || string.strcasecmp("DISABLE") == 0 ||
      string.strcasecmp("DISABLED") == 0
  ;
  r = a1 || a2;
  value = a1;
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::isBooleanString(const utf8::String & string, intmax_t * i)
{
  bool  r, v;
  r = isBooleanString(string, v);
  if( i != NULL ) *i = v;
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::isIntegerString(const utf8::String & string, intmax_t & value)
{
  utf8::String::Iterator i(string);
  while( i.isSpace() && i.next() );
  while( i.isDigit() && i.next() );
  if( i.eof() ) return false;
  switch( i.getUpperChar() ){
    case 'K' :
      value = utf8::str2Int(utf8::String(utf8::String::Iterator(string), i)) * 1024u;
      i.next();
      break;
    case 'M' :
      value = utf8::str2Int(utf8::String(utf8::String::Iterator(string), i)) * 1024u * 1024u;
      i.next();
      break;
    case 'G' :
      value = utf8::str2Int(utf8::String(utf8::String::Iterator(string), i)) * 1024u * 1024u * 1024u;
      i.next();
      break;
  }
  while( i.isSpace() && i.next() );
  return i.eof();
}
//---------------------------------------------------------------------------
bool Mutant::isIntegerString(const utf8::String & string, intmax_t * i)
{
  bool      r;
  intmax_t  v;
  r = isIntegerString(string, v);
  if( i != NULL ) *i = v;
  return r;
}
//---------------------------------------------------------------------------
Mutant & Mutant::changeType(MutantType newType, const Mutant & m)
{
  if( type_ != newType || this != &m ){
    union {
        intmax_t        i;
#if HAVE_LONG_DOUBLE
        long
        #endif
        double f;
        const char *    cStr;
        const wchar_t * wStr;
        const char *    str;
    };
    utf8::String  s;
    MemoryStream  ms;
    switch( newType ){
      case      mtNull :
        clear().type_ = mtNull;
        break;
      case       mtInt :
        switch( m.type_ ){
          case      mtNull :
            clear().int_ = 0;
            break;
          case       mtInt :
          case      mtTime :
            i = m.int_;
            clear().int_ = i;
            break;
          case     mtFloat :
            i = (intmax_t) m.float_;
            clear().int_ = i;
            break;
          case      mtCStr :
            if( !isBooleanString(m.cStr_, &i) && !isIntegerString(m.cStr_, &i) )
              i = utf8::str2Int(m.cStr_);
            clear().int_ = i;
            break;
          case      mtWStr :
            if( !isBooleanString(m.wStr_, &i) && !isIntegerString(m.cStr_, &i) )
              i = utf8::str2Int(m.wStr_);
            clear().int_ = i;
            break;
          case       mtStr :
            if( !isBooleanString(utf8::plane(m.str_), &i) && !isIntegerString(utf8::plane(m.str_), &i) )
              i = utf8::str2Int(m.str_);
            clear().int_ = i;
            break;
          case    mtString :
            if( !isBooleanString(m.string(), &i) && !isIntegerString(m.string(), &i) )
              i = utf8::str2Int(m.string());
            clear().int_ = i;
            break;
          case    mtBinary :
            throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        }
        break;
      case      mtTime :
        switch( m.type_ ){
          case      mtNull :
            clear().int_ = 0;
            break;
          case       mtInt :
          case      mtTime :
            i = m.int_;
            clear().int_ = i;
            break;
          case     mtFloat :
            i = (intmax_t) m.float_;
            clear().int_ = i;
            break;
          case      mtCStr :
            i = utf8::str2Time(m.cStr_);
            clear().int_ = i;
            break;
          case      mtWStr :
            i = utf8::str2Time(m.wStr_);
            clear().int_ = i;
            break;
          case       mtStr :
            i = utf8::str2Time(m.str_);
            clear().int_ = i;
            break;
          case    mtString :
            i = utf8::str2Time(m.string());
            clear().int_ = i;
            break;
          case    mtBinary :
            throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        }
        break;
      case     mtFloat :
        switch( m.type_ ){
          case      mtNull :
            clear().float_ = 0;
            break;
          case       mtInt :
          case      mtTime :
#if HAVE_LONG_DOUBLE
            f = (long double) m.int_;
#else
            f = (long double) m.int_;
#endif
            clear().float_ = f;
            break;
          case     mtFloat :
            f = m.float_;
            clear().float_ = f;
            break;
          case      mtCStr :
            f = utf8::str2Float(m.cStr_);
            clear().float_ = f;
            break;
          case      mtWStr :
            f = utf8::str2Float(m.wStr_);
            clear().float_ = f;
            break;
          case       mtStr :
            f = utf8::str2Float(m.str_);
            clear().float_ = f;
            break;
          case    mtString :
            f = utf8::str2Float(m.string());
            clear().float_ = f;
            break;
          case    mtBinary :
            throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        }
        break;
      case      mtCStr :
        if( m.type_ != mtCStr )
          throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        cStr = m.cStr_;
        clear().cStr_ = cStr;
        break;
      case      mtWStr :
        if( m.type_ != mtWStr )
          throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        wStr = m.wStr_;
        clear().wStr_ = wStr;
        break;
      case       mtStr :
        if( m.type_ != mtStr )
          throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        str = m.str_;
        clear().str_ = str;
        break;
      case    mtString :
        switch( m.type_ ){
          case      mtNull :
            new (clear().raw_) utf8::String;
            break;
          case       mtInt :
            s = utf8::int2Str(m.int_);
            new (clear().raw_) utf8::String(s);
            break;
          case      mtTime :
            s = utf8::time2Str(m.int_);
            new (clear().raw_) utf8::String(s);
            break;
          case     mtFloat :
            s = utf8::float2Str(m.float_);
            new (clear().raw_) utf8::String(s);
            break;
          case      mtCStr :
            s = utf8::String(m.cStr_);
            new (clear().raw_) utf8::String(s);
            break;
          case      mtWStr :
            s = utf8::String(m.wStr_);
            new (clear().raw_) utf8::String(s);
            break;
          case       mtStr :
            s = utf8::plane(m.str_);
            new (clear().raw_) utf8::String(s);
            break;
          case    mtString :
            s = utf8::String(m.string());
            new (clear().raw_) utf8::String(s);
            break;
          case    mtBinary :
            throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        }
        break;
      case    mtBinary :
        if( m.type_ != mtBinary )
          throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
        ms = m.stream();
        clear().stream() = ms;
    }
    type_ = newType;
  }
  return *this;
}
//---------------------------------------------------------------------------
Mutant::operator bool() const
{
  bool  v;
  switch( type_ ){
    case mtNull   :
      return false;
    case mtInt    :
      return int_ != 0;
    case mtFloat  :
      return float_ != 0;
    case mtTime   :
      return int_ != 0;
    case mtCStr   :
      if( isBooleanString(cStr_, v) )
        return v;
      break;
    case mtWStr   :
      if( isBooleanString(cStr_, v) )
        return v;
      break;
    case mtStr    :
      if( isBooleanString(utf8::plane(str_), v) )
        return v;
      break;
    case mtString :
      if( isBooleanString(string(), v) )
        return v;
      break;
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator char() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (char) int_;
    case mtFloat  :
      return (char) float_;
    case mtTime   :
      return (char) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (char) v;
      return (char) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (char) v;
      return (char) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (char) v;
      return (char) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (char) v;
      return (char) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator unsigned char() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (unsigned char) int_;
    case mtFloat  :
      return (unsigned char) float_;
    case mtTime   :
      return (unsigned char) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (unsigned char) v;
      return (unsigned char) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (unsigned char) v;
      return (unsigned char) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (unsigned char) v;
      return (unsigned char) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (unsigned char) v;
      return (unsigned char) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator short() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (short) int_;
    case mtFloat  :
      return (short) float_;
    case mtTime   :
      return (short) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (short) v;
      return (short) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (short) v;
      return (short) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (short) v;
      return (short) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (short) v;
      return (short) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator unsigned short() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (unsigned short) int_;
    case mtFloat  :
      return (unsigned short) float_;
    case mtTime   :
      return (unsigned short) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (unsigned short) v;
      return (unsigned short) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (unsigned short) v;
      return (unsigned short) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (unsigned short) v;
      return (unsigned short) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (unsigned short) v;
      return (unsigned short) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT
//---------------------------------------------------------------------------
Mutant::operator int() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (int) int_;
    case mtFloat  :
      return (int) float_;
    case mtTime   :
      return (int) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (int) v;
      return (int) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (int) v;
      return (int) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (int) v;
      return (int) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (int) v;
      return (int) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator unsigned() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (unsigned int) int_;
    case mtFloat  :
      return (unsigned int) float_;
    case mtTime   :
      return (unsigned int) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (unsigned int) v;
      return (unsigned int) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (unsigned int) v;
      return (unsigned int) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (unsigned int) v;
      return (unsigned int) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (unsigned int) v;
      return (unsigned int) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
Mutant::operator long() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (long) int_;
    case mtFloat  :
      return (long) float_;
    case mtTime   :
      return (long) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (long) v;
      return (long) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (long) v;
      return (long) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (long) v;
      return (long) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (long) v;
      return (long) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator unsigned long() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (unsigned long) int_;
    case mtFloat  :
      return (unsigned long) float_;
    case mtTime   :
      return (unsigned long) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (unsigned long) v;
      return (unsigned long) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (unsigned long) v;
      return (unsigned long) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (unsigned long) v;
      return (unsigned long) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (unsigned long) v;
      return (unsigned long) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator intptr_t() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (intptr_t) int_;
    case mtFloat  :
      return (intptr_t) float_;
    case mtTime   :
      return (intptr_t) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (intptr_t) v;
      return (intptr_t) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (intptr_t) v;
      return (intptr_t) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (intptr_t) v;
      return (intptr_t) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (intptr_t) v;
      return (intptr_t) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator uintptr_t() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (uintptr_t) int_;
    case mtFloat  :
      return (uintptr_t) float_;
    case mtTime   :
      return (uintptr_t) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (uintptr_t) v;
      return (uintptr_t) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (uintptr_t) v;
      return (uintptr_t) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (uintptr_t) v;
      return (uintptr_t) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (uintptr_t) v;
      return (uintptr_t) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
#if !HAVE_INTPTR_T_AS_INT64_T
Mutant::operator intmax_t() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (intmax_t) int_;
    case mtFloat  :
      return (intmax_t) float_;
    case mtTime   :
      return (intmax_t) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (intmax_t) v;
      return (intmax_t) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (intmax_t) v;
      return (intmax_t) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (intmax_t) v;
      return (intmax_t) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (intmax_t) v;
      return (intmax_t) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator uintmax_t() const
{
  intmax_t  v;
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (uintmax_t) int_;
    case mtFloat  :
      return (uintmax_t) float_;
    case mtTime   :
      return (uintmax_t) int_;
    case mtCStr   :
      if( isIntegerString(cStr_, v) )
        return (uintmax_t) v;
      return (uintmax_t) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) )
        return (uintmax_t) v;
      return (uintmax_t) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) )
        return (uintmax_t) v;
      return (uintmax_t) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) )
        return (uintmax_t) v;
      return (uintmax_t) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
#endif
//---------------------------------------------------------------------------
Mutant::operator float() const
{
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (float) int_;
    case mtFloat  :
      return (float) float_;
    case mtTime   :
      return (float) int_;
    case mtCStr   :
      return (float) utf8::str2Float(cStr_);
    case mtWStr   :
      return (float) utf8::str2Float(wStr_);
    case mtStr    :
      return (float) utf8::str2Float(utf8::plane(str_));
    case mtString :
      return (float) utf8::str2Float(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator double() const
{
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (double) int_;
    case mtFloat  :
      return (double) float_;
    case mtTime   :
      return (double) int_;
    case mtCStr   :
      return (double) utf8::str2Float(cStr_);
    case mtWStr   :
      return (double) utf8::str2Float(wStr_);
    case mtStr    :
      return (double) utf8::str2Float(utf8::plane(str_));
    case mtString :
      return (double) utf8::str2Float(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
#if HAVE_LONG_DOUBLE
Mutant::operator long double() const
{
  switch( type_ ){
    case mtNull   :
      return 0;
    case mtInt    :
      return (long double) int_;
    case mtFloat  :
      return (long double) float_;
    case mtTime   :
      return (long double) int_;
    case mtCStr   :
      return (long double) utf8::str2Float(cStr_);
    case mtWStr   :
      return (long double) utf8::str2Float(wStr_);
    case mtStr    :
      return (long double) utf8::str2Float(utf8::plane(str_));
    case mtString :
      return (long double) utf8::str2Float(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
#endif
//---------------------------------------------------------------------------
Mutant::operator struct timeval() const
{
  struct timeval  tv;
  switch( type_ ){
    case mtNull   :
      tv.tv_sec = 0;
      tv.tv_usec = 0;
      return tv;
    case mtInt    :
      return time2Timeval(int_);
    case mtFloat  :
      return time2Timeval((int64_t) float_);
    case mtTime   :
      return time2Timeval(int_);
    case mtCStr   :
      return utf8::str2Timeval(cStr_);
    case mtWStr   :
      return utf8::str2Timeval(wStr_);
    case mtStr    :
      return utf8::str2Timeval(utf8::plane(str_));
    case mtString :
      return utf8::str2Timeval(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator struct tm() const
{
  struct tm tv;
  switch( type_ ){
    case mtNull   :
      memset(&tv, 0, sizeof(tv));
      return tv;
    case mtInt    :
      return time2tm(int_);
    case mtFloat  :
      return time2tm((int64_t) float_);
    case mtTime   :
      return time2tm(int_);
    case mtCStr   :
      return utf8::str2tm(cStr_);
    case mtWStr   :
      return utf8::str2tm(wStr_);
    case mtStr    :
      return utf8::str2tm(utf8::plane(str_));
    case mtString :
      return utf8::str2tm(string());
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator utf8::String() const
{
  switch( type_ ){
    case mtNull   :
      return utf8::String();
    case mtInt    :
      return utf8::int2Str(int_);
    case mtFloat  :
      return utf8::float2Str(float_);
    case mtTime   :
      return utf8::time2Str(int_);
    case mtCStr   :
      return cStr_;
    case mtWStr   :
      return wStr_;
    case mtStr    :
      return utf8::plane(str_);
    case mtString :
      return string();
    case mtBinary :
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
Mutant::operator MemoryStream() const
{
  switch( type_ ){
    case mtNull   :
      break;
    case mtInt    :
      break;
    case mtFloat  :
      break;
    case mtTime   :
      break;
    case mtCStr   :
      break;
    case mtWStr   :
      break;
    case mtStr    :
      break;
    case mtString :
      break;
    case mtBinary :
      return stream();
    default       :
      ;
  }
  throw ExceptionSP(newObject<EMutant>(EINVAL, __PRETTY_FUNCTION__));
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

