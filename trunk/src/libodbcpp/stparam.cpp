/*-
 * Copyright 2007-2008 Guram Dukashvili
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
#include <adicpp/odbcpp.h>
#if HAVE_SQL_H
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#else
typedef int SQLINTEGER;
struct TIMESTAMP_STRUCT {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int fraction;
};
#endif
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ksys::Mutant DSQLParam::getMutant()
{
  switch( type_ ){
    case ksys::mtNull   :
      break;
    case ksys::mtInt    :
      return int_;
    case ksys::mtFloat  :
      return float_;
    case ksys::mtTime   :
      return ksys::Mutant(int_).changeType(ksys::mtTime);
    case ksys::mtCStr   :
    case ksys::mtWStr   :
    case ksys::mtStr    :
    case ksys::mtString :
      return utf8::String(string_);
    case ksys::mtBinary :
      return stream_;
  }
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setMutant(const ksys::Mutant & value)
{
  struct tm t;
  TIMESTAMP_STRUCT * ts;
  switch( value.type() ){
    case ksys::mtNull   :
      break;
    case ksys::mtInt    :
      int_ = value;
      break;
    case ksys::mtFloat  :
      float_ = value;
      break;
    case ksys::mtTime   :
      t = value;
      ts = (TIMESTAMP_STRUCT *) time_;
      ts->year = uint16_t(t.tm_year + 1900);
      ts->month = uint16_t(t.tm_mon + 1);
      ts->day = uint16_t(t.tm_mday);
      ts->hour = uint16_t(t.tm_hour);
      ts->minute = uint16_t(t.tm_min);
      ts->second = uint16_t(t.tm_sec);
      ts->fraction = SQLINTEGER((uint64_t(value) % 1000000u) * 1000u);
      break;
    case ksys::mtCStr   :
    case ksys::mtWStr   :
    case ksys::mtStr    :
    case ksys::mtString :
      string_ = utf8::String(value).getUNICODEString().ptr(NULL);
      break;
    case ksys::mtBinary :
      stream_ = value;
      break;
  }
  type_ = value.type();
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLParam::getString()
{
  switch( type_ ){
    case ksys::mtNull   :
      break;
    case ksys::mtInt    :
      return utf8::int2Str(int_);
    case ksys::mtFloat  :
      return utf8::float2Str(float_);
    case ksys::mtTime   :
      return utf8::time2Str(int_);
    case ksys::mtCStr   :
    case ksys::mtWStr   :
    case ksys::mtStr    :
    case ksys::mtString :
      return utf8::String(string_);
    case ksys::mtBinary :
      break;
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setString(const utf8::String & value)
{
  string_ = value.getUNICODEString().ptr(NULL);
  type_ = ksys::mtString;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
