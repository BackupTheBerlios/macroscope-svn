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
#include <adicpp/fbcpp.h>
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ksys::Mutant DSQLParam::getMutant()
{
  newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setMutant(const ksys::Mutant &)
{
  newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
utf8::String DSQLParam::getString()
{
  newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setString(const utf8::String &)
{
  newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParamScalar::DSQLParamScalar()
{
}
//---------------------------------------------------------------------------
DSQLParamScalar::~DSQLParamScalar()
{
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParamScalar::getMutant()
{
  if( sqlind_ < 0 )
    return ksys::Mutant();
#if HAVE_LONG_DOUBLE
  long
  #endif
  double v;  
  switch( sqltype_ & ~1 ){
    case SQL_SHORT       :
      if( sqlscale_ != 0 ){
        v = (short) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v /= 10;
        return v;
      }
      return (short) bigInt_;
    case SQL_LONG        :
      if( sqlscale_ != 0 ){
        v = (int) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v /= 10;
        return v;
      }
      return (int) bigInt_;
    case SQL_FLOAT       :
      return float_;
    case SQL_DOUBLE      :
    case SQL_D_FLOAT     :
      return double_;
    case SQL_TYPE_TIME   :
    case SQL_TYPE_DATE   :
    case SQL_TIMESTAMP   :
      return iscTimeStamp2Timeval(timeStamp_);
    case SQL_QUAD        :
    case SQL_INT64       :
      if( sqlscale_ != 0 ){
#if HAVE_LONG_DOUBLE
        v = (long double) bigInt_;
#else
        v = (double) bigInt_;
#endif
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v /= 10;
        return v;
      }
      return bigInt_;
  }
  newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParamScalar::setMutant(const ksys::Mutant & value)
{
  if( value.type() == ksys::mtNull ){
    sqlind_ = -1;
  }
  else{
    switch( sqltype_ & ~1 ){
      case SQL_SHORT       :
      case SQL_LONG        :
      case SQL_QUAD        :
      case SQL_INT64       :
        if( sqlscale_ != 0 && value.type() == ksys::mtFloat ){
#if HAVE_LONG_DOUBLE
          long
          #endif
          double v  = value;
          for( long j = sqlscale_; j < 0; j++ )
            v *= 10;
          bigInt_ = (int64_t) v;
        }
        else{
          bigInt_ = value;
        }
        break;
      case SQL_FLOAT       :
        float_ = value;
        break;
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
        double_ = value;
        break;
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
        timeStamp_ = timeval2IscTimeStamp(value);
        break;
      default              :
        newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
    }
    sqlind_ = 0;
  }
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLParamScalar::getString()
{
#if HAVE_LONG_DOUBLE
  long double v;
#else
  double      v;
#endif
  if( sqlind_ >= 0 ){
    switch( sqltype_ & ~1 ){
      case SQL_SHORT       :
      case SQL_LONG        :
      case SQL_QUAD        :
      case SQL_INT64       :
        if( sqlscale_ != 0 ){
#if HAVE_LONG_DOUBLE
          v = (long double) bigInt_;
#else
          v = (double) bigInt_;
#endif
          for( intptr_t j = sqlscale_; j < 0; j++ )
            v /= 10;
          return ksys::Mutant(v);
        }
        return utf8::int2Str(bigInt_);
      case SQL_FLOAT       :
        return utf8::float2Str(float_);
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
        return utf8::float2Str(double_);
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
        return utf8::time2Str(iscTimeStamp2Time(timeStamp_));
      default              :
        newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
    }
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParamScalar::setString(const utf8::String & value)
{
  switch( sqltype_ & ~1 ){
    case SQL_SHORT       :
    case SQL_LONG        :
    case SQL_QUAD        :
    case SQL_INT64       :
      if( !utf8::tryStr2Int(value, bigInt_) ){
#if HAVE_LONG_DOUBLE
        long
        #endif
        double v  = ksys::Mutant(value);
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v *= 10;
        bigInt_ = (int64_t) v;
      }
      break;
    case SQL_FLOAT       :
      float_ = (float) utf8::str2Float(value);
      break;
    case SQL_DOUBLE      :
    case SQL_D_FLOAT     :
      double_ = (double) utf8::str2Float(value);
      break;
    case SQL_TYPE_TIME   :
    case SQL_TYPE_DATE   :
    case SQL_TIMESTAMP   :
      timeStamp_ = time2IscTimeStamp(utf8::str2Time(value));
      break;
    default              :
      newObject<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  }
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParamText::DSQLParamText()
{
}
//---------------------------------------------------------------------------
DSQLParamText::~DSQLParamText()
{
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParamText::getMutant()
{
  if( sqlind_ < 0 )
    return ksys::Mutant();
  return text_;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParamText::setMutant(const ksys::Mutant & value)
{
  if( value.type() == ksys::mtNull ){
    text_.resize(0);
    sqlind_ = -1;
  }
  else{
    text_ = value;
    sqlind_ = 0;
  }
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLParamText::getString()
{
  return text_;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParamText::setString(const utf8::String & value)
{
  text_ = value;
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParamArray::DSQLParamArray(DSQLStatement & statement)
  : statement_(&statement),
    data_(NULL)
{
  id_.gds_quad_low = 0;
  id_.gds_quad_high = 0;
}
//---------------------------------------------------------------------------
DSQLParamArray::~DSQLParamArray()
{
  ksys::xfree(data_);
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParamArray::clear()
{
  ksys::xfree(data_);
  data_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParamArray::checkData()
{
  if( data_ == NULL ){
    intptr_t  i;
    dataSize_ = desc_.array_desc_length;
    if( desc_.array_desc_dtype == blr_varying )
      dataSize_ += 2;
    elementSize_ = dataSize_;
    for( i = desc_.array_desc_dimensions - 1; i >= 0; i-- ){
      dimElements_[i] = desc_.array_desc_bounds[i].array_bound_upper - desc_.array_desc_bounds[i].array_bound_lower + 1;
      dataSize_ *= dimElements_[i];
    }
    memmove(dimElements_, dimElements_ + 1, sizeof(dimElements_) - sizeof(dimElements_[0]));
    dimElements_[desc_.array_desc_dimensions - 1] = 1;
    for( i = desc_.array_desc_dimensions - 1; i > 0; i-- ){
      dimElements_[i - 1] = uint16_t(dimElements_[i - 1] * dimElements_[i]);
    }
    ksys::xmalloc(data_, dataSize_);
    memset(data_, 0, dataSize_);
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParamArray::checkDim(bool inRange)
{
  if( !inRange )
    newObject<EDSQLStInvalidArrayDim>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParamArray::setDataFromMutant(uintptr_t absIndex, const ksys::Mutant & value)
{
  checkData();
  uintptr_t elementOffset = elementSize_ * absIndex, len ;
  if( elementOffset >= (uintptr_t) dataSize_ )
    newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  char * data  = (char *) data_ + elementOffset;
  utf8::String  tempString;
#if HAVE_LONG_DOUBLE
  long
  #endif
  double v;
  switch( desc_.array_desc_dtype ){
    case blr_varying   :
      tempString = value;
      len = tempString.size();
      if( len > uintptr_t(elementSize_ - 2) )
        len = elementSize_ - 2;
      *(int16_t *) (data + elementSize_ - 2) = (int16_t) len;
      memcpy(data, tempString.c_str(), len);
      break;
    case blr_text      :
      tempString = value;
      if( tempString.size() < (uintptr_t) elementSize_ ){
        memcpy(data, tempString.c_str(), tempString.size());
        memset(data + tempString.size(), ' ', elementSize_ - tempString.size());
      }
      else{
        memcpy(data, tempString.c_str(), elementSize_);
      }
      break;
    case blr_short     :
      if( desc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
          v *= 10;
        *(int16_t *) data = (int16_t) v;
      }
      else{
        *(int16_t *) data = value;
      }
      break;
    case blr_long      :
      if( desc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
          v *= 10;
        *(int32_t *) data = (int32_t) v;
      }
      else{
        *(int32_t *) data = (int32_t) value;
      }
      break;
    case blr_float     :
      *(float *) data = value;
      break;
    case blr_double    :
    case blr_d_float   :
      *(double *) data = value;
      break;
    case blr_sql_time  :
    case blr_sql_date  :
    case blr_timestamp :
      *(ISC_TIMESTAMP *) data = timeval2IscTimeStamp(value);
      break;
    case blr_quad      :
    case blr_int64     :
      if( desc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
          v *= 10;
        *(int64_t *) data = (int64_t) v;
      }
      else{
        *(int64_t *) data = value;
      }
      break;
    default            :
      newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParamArray::getMutantFromArray(uintptr_t absIndex)
{
  if( sqlind_ < 0 )
    return ksys::Mutant();
  checkData();
  uintptr_t   elementOffset = elementSize_ * absIndex;
#if HAVE_LONG_DOUBLE
  long double v;
#else
  double      v;
#endif
  if( elementOffset < (uintptr_t) dataSize_ ){
    char *  data  = (char *) data_ + elementOffset;
    switch( desc_.array_desc_dtype ){
      case blr_varying   :
        return utf8::plane(data + sizeof(short), *(short *) data);
      case blr_text      :
        return utf8::plane(data, elementSize_);
      case blr_short     :
        if( desc_.array_desc_scale != 0 ){
          v = *(int16_t *) data;
          for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
            v /= 10;
          return v;
        }
        return *(int16_t *) data;
      case blr_long      :
        if( desc_.array_desc_scale != 0 ){
          v = *(int32_t *) data;
          for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
            v /= 10;
          return v;
        }
        return *(int32_t *) data;
      case blr_float     :
        return *(float *) data;
      case blr_double    :
      case blr_d_float   :
        return *(double *) data;
      case blr_sql_time  :
      case blr_sql_date  :
      case blr_timestamp :
        return iscTimeStamp2Timeval(*(ISC_TIMESTAMP *) data);
      case blr_quad      :
      case blr_int64     :
        if( desc_.array_desc_scale != 0 ){
#if HAVE_LONG_DOUBLE
          v = (long double) * (int64_t *) data;
#else
          v = (double) *(int64_t *) data;
#endif
          for( intptr_t j = desc_.array_desc_scale; j < 0; j++ )
            v /= 10;
          return v;
        }
        return *(int64_t *) data;
    }
  }
  newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParamArray::putSlice()
{
  if( sqlind_ >= 0 )
    statement_->arrayPutSlice(id_, desc_, data_, dataSize_);
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParamBlob::DSQLParamBlob(DSQLStatement & statement)
  : statement_(&statement),
    handle_(0)
{
  id_.gds_quad_low = 0;
  id_.gds_quad_high = 0;
}
//---------------------------------------------------------------------------
DSQLParamBlob::~DSQLParamBlob()
{
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParamBlob::setString(const utf8::String & value)
{
  cancelBlob().createBlob().writeBuffer(value.c_str(), value.size()).closeBlob();
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParamBlob::createBlob()
{
  statement_->createBlob(handle_, id_, 0, NULL);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParamBlob::closeBlob()
{
  statement_->closeBlob(handle_);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParamBlob::cancelBlob()
{
  statement_->cancelBlob(handle_);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParamBlob::writeBuffer(const void * buf, uintptr_t size)
{
  while( size > 0 ){
    uintptr_t         asize = size > 32767 ? 32767 : size;
    ISC_STATUS_ARRAY  status;
    if( api.isc_put_segment(status, &handle_, (short) asize, (char *) buf) != 0 )
      statement_->database_->exceptionHandler(
        newObjectV<EDSQLStPutSegment>(status, __PRETTY_FUNCTION__));
    buf = (const char *) buf + asize;
    size -= asize;
  }
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParams::DSQLParams(DSQLStatement & statement)
  : statement_(&statement),
    changed_(false)
{
  params_.caseSensitive(false);
}
//---------------------------------------------------------------------------
DSQLParams::~DSQLParams()
{
}
//---------------------------------------------------------------------------
DSQLParamText * DSQLParams::castParamToText(DSQLParam * param)
{
  DSQLParamText * newParam;
  if( (newParam = dynamic_cast< DSQLParamText *>(param)) == NULL ){
    newParam = newObject<DSQLParamText>();
    DSQLParamScalar * paramScalar;
    if( (paramScalar = dynamic_cast< DSQLParamScalar *>(param)) != NULL )
      newParam->setMutant(paramScalar->getMutant());
    params_.changeObject(param, newParam);
    newParam->sqlind_ = -1;
  }
  return newParam;
}
//---------------------------------------------------------------------------
DSQLParamScalar * DSQLParams::castParamToScalar(DSQLParam * param)
{
  DSQLParamScalar * newParam;
  if( (newParam = dynamic_cast< DSQLParamScalar *>(param)) == NULL ){
    newParam = newObject<DSQLParamScalar>();
    DSQLParamText * paramText;
    if( (paramText = dynamic_cast< DSQLParamText *>(param)) != NULL )
      newParam->setMutant(paramText->getMutant());
    params_.changeObject(param, newParam);
  }
  return newParam;
}
//---------------------------------------------------------------------------
DSQLParamArray * DSQLParams::castParamToArray(DSQLParam * param, XSQLVAR & v)
{
  DSQLParamArray *  newParam;
  if( (newParam = dynamic_cast< DSQLParamArray *>(param)) == NULL ){
    newParam = newObjectV<DSQLParamArray>(*statement_);
    try {
      statement_->arrayLookupBounds(v.relname, v.sqlname, newParam->desc_);
    }
    catch( ksys::ExceptionSP & ){
      delete newParam;
      throw;
    }
    params_.changeObject(param, newParam);
    newParam->sqlind_ = -1;
  }
  return newParam;
}
//---------------------------------------------------------------------------
DSQLParamBlob * DSQLParams::castParamToBlob(DSQLParam * param)
{
  DSQLParamBlob * newParam;
  if( (newParam = dynamic_cast< DSQLParamBlob *>(param)) == NULL ){
    params_.changeObject(param, newParam = newObjectV<DSQLParamBlob>(*statement_));
    newParam->handle_ = 0;
    newParam->id_.gds_quad_low = 0;
    newParam->id_.gds_quad_high = 0;
    newParam->sqlind_ = -1;
  }
  return newParam;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::bind(BindType bindType_)
{
  union {
      DSQLParam *       param;
      DSQLParamText *   paramText;
      DSQLParamScalar * paramScalar;
      DSQLParamArray *  paramArray;
      DSQLParamBlob *   paramBlob;
  };
  for( intptr_t j, i = indexToParam_ .count() - 1; i >= 0; i-- ){
    param = indexToParam_.ptr()[i]->object();
    XSQLVAR & v = sqlda_.sqlda()->sqlvar[i];
    switch( v.sqltype & ~1 ){
      case SQL_VARYING     :
      case SQL_TEXT        :
        if( bindType_ == postBind ){
          if( paramText->sqlind_ == 0 ){
            v.sqltype = (int16_t) (SQL_TEXT | (v.sqltype & 1));
            v.sqllen = (int16_t) paramText->text_.size();
            v.sqldata = paramText->text_.c_str();
          }
          else{
            paramText->text_.resize(0);
          }
        }
        else{
          paramText = castParamToText(param);
        }
        break;
      case SQL_SHORT       :
      case SQL_LONG        :
        if( bindType_ == postBind ){
          if( paramScalar->sqlind_ == 0 ){
            v.sqldata = (char *) &paramScalar->bigInt_;
          }
        }
        else{
          paramScalar = castParamToScalar(param);
        }
        break;
      case SQL_FLOAT       :
        if( bindType_ == postBind ){
          if( paramScalar->sqlind_ == 0 ){
            v.sqldata = (char *) &paramScalar->float_;
            for( j = v.sqlscale; j < 0; j++ )
              paramScalar->float_ *= 10;
          }
        }
        else{
          paramScalar = castParamToScalar(param);
        }
        break;
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
        if( bindType_ == postBind ){
          if( paramScalar->sqlind_ == 0 ){
            v.sqldata = (char *) &paramScalar->double_;
            for( j = v.sqlscale; j < 0; j++ )
              paramScalar->double_ *= 10;
          }
        }
        else{
          paramScalar = castParamToScalar(param);
        }
        break;
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
        if( bindType_ == postBind ){
          if( paramScalar->sqlind_ == 0 ){
            v.sqldata = (char *) &paramScalar->timeStamp_;
          }
        }
        else{
          paramScalar = castParamToScalar(param);
        }
        break;
      case SQL_QUAD        :
      case SQL_INT64       :
        if( bindType_ == postBind ){
          if( paramScalar->sqlind_ == 0 ){
            v.sqldata = (char *) &paramScalar->bigInt_;
          }
        }
        else{
          paramScalar = castParamToScalar(param);
        }
        break;
      case SQL_BLOB :
        if( bindType_ == postBind ){
          if( paramBlob->sqlind_ < 0 )
            paramBlob->cancelBlob();
          else
            paramBlob->closeBlob();
        }
        else{
          paramBlob = castParamToBlob(param);
          paramBlob->cancelBlob();
          v.sqldata = (char *) &paramBlob->id_;
          paramBlob->sqlind_ = -1;
        }
        break;
      case SQL_ARRAY :
        if( bindType_ == postBind ){
          paramArray->putSlice();
        }
        else{
          paramArray = castParamToArray(param, v);
          if( bindType_ != preBindAfterExecute )
            paramArray->clear();
          paramArray->id_.gds_quad_low = 0;
          paramArray->id_.gds_quad_high = 0;
          v.sqldata = (char *) &paramArray->id_;
        }
    }
    if( bindType_ == preBind ){
      v.sqlind = &param->sqlind_;
      param->sqltype_ = v.sqltype;
      param->sqlscale_ = (char) v.sqlscale;
      indexToParam_.ptr()[i]->object() = param;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLParams::isNull(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_.ptr()[i]->object()->sqlind_ < 0 ? true : false;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::setNull(uintptr_t i)
{
  union {
      DSQLParam *       param;
      DSQLParamBlob *   paramBlob;
      DSQLParamArray *  paramArray;
  };
  param = checkParamIndex(i).indexToParam_.ptr()[i]->object();
  param->sqlind_ = -1;
  if( dynamic_cast< DSQLParamBlob *>(param) != NULL ){
    paramBlob->cancelBlob();
  }
  else if( dynamic_cast< DSQLParamArray *>(param) != NULL ){
    paramArray->clear();
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLParams::isNull(const utf8::String & paramName)
{
  return checkParamName(paramName)->sqlind_ < 0 ? true : false;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::setNull(const utf8::String & paramName)
{
  union {
      DSQLParam *       param;
      DSQLParamBlob *   paramBlob;
      DSQLParamArray *  paramArray;
  };
  param = checkParamName(paramName);
  param->sqlind_ = -1;
  if( dynamic_cast< DSQLParamBlob *>(param) != NULL ){
    paramBlob->cancelBlob();
  }
  else if( dynamic_cast< DSQLParamArray *>(param) != NULL ){
    paramArray->clear();
  }
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParams::asMutant(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_.ptr()[i]->object()->getMutant();
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParams::asMutant(const utf8::String & paramName)
{
  return checkParamName(paramName)->getMutant();
}
//---------------------------------------------------------------------------
utf8::String DSQLParams::asString(uintptr_t i)
{
  return checkParamIndex(i).indexToParam_.ptr()[i]->object()->getString();
}
//---------------------------------------------------------------------------
utf8::String DSQLParams::asString(const utf8::String & paramName)
{
  return checkParamName(paramName)->getString();
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asMutant(uintptr_t i, const ksys::Mutant & value)
{
  checkParamIndex(i).indexToParam_.ptr()[i]->object()->setMutant(value);
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
  checkParamIndex(i).indexToParam_.ptr()[i]->object()->setString(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::asString(const utf8::String & paramName, const utf8::String & value)
{
  checkParamName(paramName)->setString(value);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParams::asBlob(uintptr_t i)
{
  DSQLParamBlob * blob  = dynamic_cast< DSQLParamBlob *>(checkParamIndex(i).indexToParam_.ptr()[i]->object());
  if( blob == NULL )
    newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *blob;
}
//---------------------------------------------------------------------------
DSQLParamBlob & DSQLParams::asBlob(const utf8::String & name)
{
  DSQLParamBlob * blob  = dynamic_cast< DSQLParamBlob *>(checkParamName(name));
  if( blob == NULL )
    newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *blob;
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParams::asArray(uintptr_t i)
{
  DSQLParamArray *  array = dynamic_cast< DSQLParamArray *>(checkParamIndex(i).indexToParam_.ptr()[i]->object());
  if( array == NULL )
    newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *array;
}
//---------------------------------------------------------------------------
DSQLParamArray & DSQLParams::asArray(const utf8::String & name)
{
  DSQLParamArray *  array = dynamic_cast< DSQLParamArray *>(checkParamName(name));
  if( array == NULL )
    newObject<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *array;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::checkParamIndex(uintptr_t i)
{
  if( i >= params_.count() )
    newObject<EDSQLStInvalidParamIndex>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam * DSQLParams::checkParamName(const utf8::String & paramName)
{
  DSQLParam * param = params_.objectOfKey(paramName);
  if( param == NULL )
    newObject<EDSQLStInvalidParamName>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return param;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::resetChanged()
{
  for( intptr_t i = params_.count() - 1; i >= 0; i-- )
    params_.objectOfIndex(i)->changed_ = false;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::removeUnchanged()
{
  for( intptr_t i = params_.count() - 1; i >= 0; i-- )
    if( !params_.objectOfIndex(i)->changed_ )
      params_.removeByIndex(i);
  return *this;
}
//---------------------------------------------------------------------------
ksys::HashedObjectListItem< utf8::String,DSQLParam> * DSQLParams::add(const utf8::String & paramName)
{
  ksys::HashedObjectListItem< utf8::String,DSQLParam> * item;
  item = params_.itemOfKey(paramName);
  if( item == NULL )
    params_.add(newObject<DSQLParam>(), paramName, &item);
  item->object()->changed_ = true;
  return item;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
