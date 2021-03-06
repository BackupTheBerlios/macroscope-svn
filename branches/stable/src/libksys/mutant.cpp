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
    string.casecompare("TRUE") == 0 || string.casecompare("YES") == 0 ||
    string.casecompare("ON") == 0 || string.casecompare("ACCEPT") == 0 ||
    string.casecompare("PASS") == 0 || string.casecompare("PERMIT") == 0 ||
    string.casecompare("ALLOW") == 0 || string.casecompare("ALLOWED") == 0 ||
    string.casecompare("ENABLE") == 0 || string.casecompare("ENABLED") == 0
  ;
  a2 =
      string.casecompare("FALSE") == 0 || string.casecompare("NO") == 0 ||
      string.casecompare("OFF") == 0 || string.casecompare("DENY") == 0 ||
      string.casecompare("DENIED") == 0 || string.casecompare("DISABLE") == 0 ||
      string.casecompare("DISABLED") == 0
  ;
  r = a1 || a2;
  value = a1 ? true : a2 ? false : false;
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
bool Mutant::isIntegerString(const utf8::String & string,intmax_t & value)
{
  utf8::String::Iterator i(string);
  while( i.isSpace() && i.next() );
  while( i.isDigit() && i.next() );
  if( i.eos() ) return false;
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
    case 'T' :
      value = utf8::str2Int(utf8::String(utf8::String::Iterator(string),i)) * uintmax_t(1024u * 1024u * 1024u) * 1024;
      i.next();
      break;
  }
  while( i.isSpace() && i.next() );
  return i.eos();
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
bool Mutant::tryChangeType(MutantType newType)
{
  bool r = true;
  try {
    changeType(newType);
  }
  catch( ExceptionSP & ){
    r = false;
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::tryChangeType(MutantType newType,const Mutant & m)
{
  bool r = true;
  try {
    changeType(newType,m);
  }
  catch( ExceptionSP & ){
    r = false;
  }
  return r;
}
//---------------------------------------------------------------------------
Mutant & Mutant::changeType(MutantType newType,const Mutant & m)
{
  if( type_ != newType || this != &m ){
    union {
      intmax_t i;
      ldouble f;
    };
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
            newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
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
            newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
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
            newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
        }
        break;
      case      mtCStr :
      case      mtWStr :
      case       mtStr :
        {
          utf8::String s(m);
          new (clear().raw_) utf8::String(s);
          newType = mtString;
        }
        break;
      case    mtString :
        {
          utf8::String s(m);
          new (clear().raw_) utf8::String(s);
        }
        break;
      case    mtBinary :
        {
          MemoryStream ms;
          ms = m;
          clear().stream() = ms;
        }
    }
    type_ = newType;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool Mutant::isBoolean(bool * pv) const
{
  bool v, is;
  intmax_t m;
  switch( type_ ){
    case mtNull   :
      v = false;
      is = true;
      break;
    case mtInt    :
      v = int_ != 0;
      is = true;
      break;
    case mtFloat  :
      v = float_ != 0;
      is = true;
      break;
    case mtTime   :
      v = int_ != 0;
      is = true;
      break;
    case mtCStr   :
      if( (is = isIntegerString(cStr_,m)) ) v = m != 0; else is = isBooleanString(cStr_,v);
      break;
    case mtWStr   :
      if( (is = isIntegerString(wStr_,m)) ) v = m != 0; else is = isBooleanString(wStr_,v);
      break;
    case mtStr    :
      if( (is = isIntegerString(utf8::plane(str_),m)) ) v = m != 0;
        else is = isBooleanString(utf8::plane(str_),v);
      break;
    case mtString :
      if( (is = isIntegerString(string(),m)) ) v = m != 0; else is = isBooleanString(string(),v);
      break;
    case mtBinary :
    default       :
      is = false;
  }
  if( pv != NULL ) *pv = v;
  return is;
}
//---------------------------------------------------------------------------
Mutant::operator bool() const
{
  bool v;
  if( isBoolean(&v) ) return v;
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
      if( isIntegerString(cStr_, v) ) return (char) v;
      return (char) utf8::str2Int(cStr_);
    case mtWStr   :
      if( isIntegerString(wStr_, v) ) return (char) v;
      return (char) utf8::str2Int(wStr_);
    case mtStr    :
      if( isIntegerString(utf8::plane(str_), v) ) return (char) v;
      return (char) utf8::str2Int(utf8::plane(str_));
    case mtString :
      if( isIntegerString(string(), v) ) return (char) v;
      return (char) utf8::str2Int(string());
    case mtBinary :
    default       :
      ;
  }
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------
#if !HAVE_LONG_AS_INTPTR_T
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
#endif
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
#if !HAVE_INTMAX_T_AS_INTPTR_T
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
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
  newObjectV1C2<EMutant>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
bool Mutant::operator == (const Mutant & m) const
{
  bool r = false;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = true; break;
      case mtInt    : r = int_ == m.int_; break;
      case mtFloat  : r = float_ == m.float_; break;
      case mtTime   : r = int_ == m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) == 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) == 0; break;
      case mtStr    : r = strcmp(str_,m.str_) == 0; break;
      case mtString : r = string().compare(m.string()) == 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) == 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::operator != (const Mutant & m) const
{
  bool r = true;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = false; break;
      case mtInt    : r = int_ != m.int_; break;
      case mtFloat  : r = float_ != m.float_; break;
      case mtTime   : r = int_ != m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) != 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) != 0; break;
      case mtStr    : r = strcmp(str_,m.str_) != 0; break;
      case mtString : r = string().compare(m.string()) != 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) != 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::operator >  (const Mutant & m) const
{
  bool r = type_ > m.type_;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = false; break;
      case mtInt    : r = int_ > m.int_; break;
      case mtFloat  : r = float_ > m.float_; break;
      case mtTime   : r = int_ > m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) > 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) > 0; break;
      case mtStr    : r = strcmp(str_,m.str_) > 0; break;
      case mtString : r = string().compare(m.string()) > 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) > 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::operator >= (const Mutant & m) const
{
  bool r = type_ > m.type_;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = true; break;
      case mtInt    : r = int_ >= m.int_; break;
      case mtFloat  : r = float_ >= m.float_; break;
      case mtTime   : r = int_ >= m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) >= 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) >= 0; break;
      case mtStr    : r = strcmp(str_,m.str_) >= 0; break;
      case mtString : r = string().compare(m.string()) >= 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) >= 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::operator <  (const Mutant & m) const
{
  bool r = type_ < m.type_;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = false; break;
      case mtInt    : r = int_ < m.int_; break;
      case mtFloat  : r = float_ < m.float_; break;
      case mtTime   : r = int_ < m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) < 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) < 0; break;
      case mtStr    : r = strcmp(str_,m.str_) < 0; break;
      case mtString : r = string().compare(m.string()) < 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) < 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
bool Mutant::operator <= (const Mutant & m) const
{
  bool r = type_ < m.type_;
  if( type_ == m.type_ ){
    switch( type_ ){
      case mtNull   : r = true; break;
      case mtInt    : r = int_ <= m.int_; break;
      case mtFloat  : r = float_ <= m.float_; break;
      case mtTime   : r = int_ <= m.int_; break;
      case mtCStr   : r = utf8::String(cStr_).compare(m.cStr_) <= 0; break;
      case mtWStr   : r = utf8::String(cStr_).compare(m.wStr_) <= 0; break;
      case mtStr    : r = strcmp(str_,m.str_) <= 0; break;
      case mtString : r = string().compare(m.string()) <= 0; break;
      case mtBinary : r = memncmp(stream().raw(),stream().count(),m.stream().raw(),m.stream().count()) <= 0; break;
      default       :
        newObjectV1C2<EMutant>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
    }
  }
  return r;
}
//---------------------------------------------------------------------------
} // namespace ksys
//---------------------------------------------------------------------------

