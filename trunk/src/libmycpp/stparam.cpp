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
#include <adicpp/mycpp.h>
//---------------------------------------------------------------------------
namespace mycpp {
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
      return string_;
    case ksys::mtBinary :
      return stream_;
  }
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setMutant(const ksys::Mutant & value)
{
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
      int_ = value;
      break;
    case ksys::mtCStr   :
    case ksys::mtWStr   :
    case ksys::mtStr    :
    case ksys::mtString :
      string_ = value;
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
      return string_;
    case ksys::mtBinary :
      break;
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setString(const utf8::String & value)
{
  string_ = value;
  type_ = ksys::mtString;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::bind()
{
  struct timeval  tv;
  struct tm       t;
  DSQLParam *     param;
  memset(bind_.bind(), 0, sizeof(MYSQL_BIND) * bind_.count());
  for( uintptr_t i = 0; i < indexToParam_.count(); i++ ){
    param = indexToParam_[i]->object();
    MYSQL_BIND & v = bind_.bind()[i];
    param->isNull_ = false;
    switch( param->type_ ){
      case ksys::mtNull :
        param->isNull_ = true;
        break;
      case ksys::mtInt :
        v.buffer_type = MYSQL_TYPE_LONGLONG;
        v.buffer = &param->int_;
        //        v.buffer_length = sizeof(int64_t);
        break;
      case ksys::mtFloat :
        v.buffer_type = MYSQL_TYPE_DOUBLE;
        v.buffer = &param->float_;
        //        v.buffer_length = sizeof(double);
        break;
      case ksys::mtTime :
        tv = ksys::time2Timeval(param->int_);
        t = ksys::timeval2tm(tv);
        param->timestamp_.year = t.tm_year + 1900;
        param->timestamp_.month = t.tm_mon + 1;
        param->timestamp_.day = t.tm_mday;
        param->timestamp_.hour = t.tm_hour;
        param->timestamp_.minute = t.tm_min;
        param->timestamp_.second = t.tm_sec;
        param->timestamp_.second_part = tv.tv_usec;
        param->timestamp_.neg = false;
        param->timestamp_.time_type = MYSQL_TIMESTAMP_DATETIME;
        v.buffer_type = MYSQL_TYPE_DATETIME;
        v.buffer = &param->timestamp_;
        //        v.buffer_length = sizeof(MYSQL_TIME);
        break;
      case ksys::mtCStr :
      case ksys::mtWStr :
      case ksys::mtStr  :
      case ksys::mtString :
        v.buffer_type = MYSQL_TYPE_STRING;
        v.buffer = param->string_.c_str();
        v.buffer_length = (unsigned long) param->string_.size();
        break;
      case ksys::mtBinary :
        v.buffer_type = MYSQL_TYPE_BLOB;
        v.buffer = param->stream_.raw();
        v.buffer_length = (unsigned long) param->stream_.count();
        break;
    }
    //    param->length_ = v.buffer_length;
    //    v.length = &param->length_;
    v.is_null = &param->isNull_;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLParams::isNull(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_[i]->object()->type_ == ksys::mtNull;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::setNull(uintptr_t i)
{
  checkParamIndex(i).indexToParam_[i]->object()->type_ = ksys::mtNull;
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLParams::isNull(const utf8::String & paramName)
{
  return checkParamName(paramName)->type_ == ksys::mtNull;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::setNull(const utf8::String & paramName)
{
  checkParamName(paramName)->type_ = ksys::mtNull;
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParams::asMutant(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_[i]->object()->getMutant();
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParams::asMutant(const utf8::String & paramName)
{
  return checkParamName(paramName)->getMutant();
}
//---------------------------------------------------------------------------
utf8::String DSQLParams::asString(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_[i]->object()->getString();
}
//---------------------------------------------------------------------------
utf8::String DSQLParams::asString(const utf8::String & paramName)
{
  return checkParamName(paramName)->getString();
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asMutant(uintptr_t i, const ksys::Mutant & value)
{
  checkParamIndex(i).indexToParam_[i]->object()->setMutant(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asMutant(const utf8::String & paramName, const ksys::Mutant & value)
{
  checkParamName(paramName)->setMutant(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asString(uintptr_t i, const utf8::String & value)
{
  checkParamIndex(i).indexToParam_[i]->object()->setString(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asString(const utf8::String & paramName, const utf8::String & value)
{
  checkParamName(paramName)->setString(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::checkParamIndex(uintptr_t i)
{
  if( i >= params_.count() )
    newObjectV1C2<EDSQLStInvalidParamIndex>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam * DSQLParams::checkParamName(const utf8::String & paramName)
{
  DSQLParam * param = params_.objectOfKey(paramName);
  if( param == NULL )
    newObjectV1C2<EDSQLStInvalidParamName>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  return param;
}
//---------------------------------------------------------------------------
ksys::HashedObjectListItem<utf8::String,DSQLParam> * DSQLParams::add(const utf8::String & paramName)
{
  ksys::HashedObjectListItem<utf8::String,DSQLParam> * item;
  item = params_.itemOfKey(paramName);
  if( item == NULL )
    params_.add(newObjectR1<DSQLParam>(*statement_), paramName, &item);
  return item;
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------

