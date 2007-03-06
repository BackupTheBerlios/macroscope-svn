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
#include <adicpp/fbcpp.h>
//---------------------------------------------------------------------------
namespace fbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParam::~DSQLParam()
{
  clear();
}
//---------------------------------------------------------------------------
DSQLParam::DSQLParam(DSQLStatement * statement,const utf8::String & name) :
  statement_(statement),
  name_(name),
  head_(NULL), next_(NULL), sqlind_(-1), sqlscale_(0)
{
  data_ = NULL;
  id_.gds_quad_low = 0;
  id_.gds_quad_high = 0;
  handle_ = 0;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParam::getMutant()
{
  if( sqlind_ < 0 ) return ksys::Mutant();
  ldouble v;  
  switch( sqltype_ & ~1 ){
    case SQL_VARYING     :
    case SQL_TEXT        :
      return text_;
    case SQL_SHORT       :
      if( sqlscale_ != 0 ){
        v = (short) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ ) v /= 10;
        return v;
      }
      return (short) bigInt_;
    case SQL_LONG        :
      if( sqlscale_ != 0 ){
        v = (int) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ ) v /= 10;
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
        v = (ldouble) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ ) v /= 10;
        return v;
      }
      return bigInt_;
    case SQL_BLOB        :
    case SQL_ARRAY       :
      return ksys::Mutant();
  }
  newObjectV1C2<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setMutant(const ksys::Mutant & value)
{
  if( next_ != NULL ) next_->setMutant(value);
  if( value.type() == ksys::mtNull ){
    sqlind_ = -1;
    return *this;
  }
  switch( sqltype_ & ~1 ){
    case SQL_VARYING     :
    case SQL_TEXT        :
      text_ = value;
      break;
    case SQL_SHORT       :
    case SQL_LONG        :
    case SQL_QUAD        :
    case SQL_INT64       :
      if( sqlscale_ != 0 && value.type() == ksys::mtFloat ){
        ldouble v = value;
        for( long j = sqlscale_; j < 0; j++ ) v *= 10;
        bigInt_ = (int64_t) v;
      }
      else {
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
    case SQL_BLOB        :
      cancelBlob().createBlob().writeBuffer(utf8::String(value).c_str(),utf8::String(value).size()).closeBlob();
      break;
    case SQL_ARRAY       :
      sqlind_ = -1;
      return *this;
    default              :
      newObjectV1C2<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  }
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLParam::getString()
{
  ldouble v;
  if( sqlind_ >= 0 ){
    switch( sqltype_ & ~1 ){
      case SQL_VARYING     :
      case SQL_TEXT        :
        return text_;
      case SQL_SHORT       :
      case SQL_LONG        :
      case SQL_QUAD        :
      case SQL_INT64       :
        if( sqlscale_ != 0 ){
          v = (ldouble) bigInt_;
          for( intptr_t j = sqlscale_; j < 0; j++ ) v /= 10;
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
      case SQL_BLOB        :
      case SQL_ARRAY       :
        break;
      default              :
        newObjectV1C2<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
    }
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setString(const utf8::String & value)
{
  if( next_ != NULL ) next_->setMutant(value);
  switch( sqltype_ & ~1 ){
    case SQL_VARYING     :
    case SQL_TEXT        :
      text_ = value;
      break;
    case SQL_SHORT       :
    case SQL_LONG        :
    case SQL_QUAD        :
    case SQL_INT64       :
      if( !utf8::tryStr2Int(value,bigInt_) ){
        ldouble v = utf8::str2Float(value);
        for( intptr_t j = sqlscale_; j < 0; j++ ) v *= 10;
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
    case SQL_BLOB        :
      cancelBlob().createBlob().writeBuffer(value.c_str(), value.size()).closeBlob();
      break;
    case SQL_ARRAY       :
      sqlind_ = -1;
      return *this;
    default              :
      newObjectV1C2<EDSQLStInvalidParamValue>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  }
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::clear()
{
  ksys::xfree(data_);
  data_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::checkData()
{
  if( data_ == NULL ){
    intptr_t  i;
    dataSize_ = arrayDesc_.array_desc_length;
    if( arrayDesc_.array_desc_dtype == blr_varying ) dataSize_ += 2;
    elementSize_ = dataSize_;
    for( i = arrayDesc_.array_desc_dimensions - 1; i >= 0; i-- ){
      dimElements_[i] = arrayDesc_.array_desc_bounds[i].array_bound_upper - arrayDesc_.array_desc_bounds[i].array_bound_lower + 1;
      dataSize_ *= dimElements_[i];
    }
    memmove(dimElements_,dimElements_ + 1,sizeof(dimElements_) - sizeof(dimElements_[0]));
    dimElements_[arrayDesc_.array_desc_dimensions - 1] = 1;
    for( i = arrayDesc_.array_desc_dimensions - 1; i > 0; i-- ){
      dimElements_[i - 1] = uint16_t(dimElements_[i - 1] * dimElements_[i]);
    }
    ksys::xmalloc(data_,dataSize_);
    memset(data_,0,dataSize_);
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::checkDim(bool inRange)
{
  if( !inRange )
    newObjectV1C2<EDSQLStInvalidArrayDim>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setDataFromMutant(uintptr_t absIndex, const ksys::Mutant & value)
{
  if( next_ != NULL ) next_->setDataFromMutant(absIndex,value);
  checkData();
  uintptr_t elementOffset = elementSize_ * absIndex, len;
  if( elementOffset >= (uintptr_t) dataSize_ )
    newObjectV1C2<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  char * data = (char *) data_ + elementOffset;
  utf8::String tempString;
  ldouble v;
  switch( arrayDesc_.array_desc_dtype ){
    case blr_varying   :
      tempString = value;
      len = tempString.size();
      if( len > uintptr_t(elementSize_ - 2) ) len = elementSize_ - 2;
      *(int16_t *) (data + elementSize_ - 2) = (int16_t) len;
      memcpy(data,tempString.c_str(), len);
      break;
    case blr_text      :
      tempString = value;
      if( tempString.size() < (uintptr_t) elementSize_ ){
        memcpy(data, tempString.c_str(), tempString.size());
        memset(data + tempString.size(), ' ', elementSize_ - tempString.size());
      }
      else{
        memcpy(data,tempString.c_str(),elementSize_);
      }
      break;
    case blr_short     :
      if( arrayDesc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ ) v *= 10;
        *(int16_t *) data = (int16_t) v;
      }
      else{
        *(int16_t *) data = value;
      }
      break;
    case blr_long      :
      if( arrayDesc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ ) v *= 10;
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
      if( arrayDesc_.array_desc_scale != 0 && value.type() == ksys::mtFloat ){
        v = value;
        for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ ) v *= 10;
        *(int64_t *) data = (int64_t) v;
      }
      else{
        *(int64_t *) data = value;
      }
      break;
    default            :
      newObjectV1C2<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  }
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLParam::getMutantFromArray(uintptr_t absIndex)
{
  if( sqlind_ < 0 ) return ksys::Mutant();
  checkData();
  uintptr_t elementOffset = elementSize_ * absIndex;
  ldouble v;
  if( elementOffset < (uintptr_t) dataSize_ ){
    char *  data  = (char *) data_ + elementOffset;
    switch( arrayDesc_.array_desc_dtype ){
      case blr_varying   :
        return utf8::plane(data + sizeof(short), *(short *) data);
      case blr_text      :
        return utf8::plane(data, elementSize_);
      case blr_short     :
        if( arrayDesc_.array_desc_scale != 0 ){
          v = *(int16_t *) data;
          for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ )  v /= 10;
          return v;
        }
        return *(int16_t *) data;
      case blr_long      :
        if( arrayDesc_.array_desc_scale != 0 ){
          v = *(int32_t *) data;
          for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ ) v /= 10;
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
        if( arrayDesc_.array_desc_scale != 0 ){
          v = (ldouble) * (int64_t *) data;
          for( intptr_t j = arrayDesc_.array_desc_scale; j < 0; j++ ) v /= 10;
          return v;
        }
        return *(int64_t *) data;
    }
  }
  newObjectV1C2<EDSQLStInvalidParam>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::writeBuffer(const void * buf,uintptr_t size)
{
  if( next_ != NULL ) next_->writeBuffer(buf,size);
  while( size > 0 ){
    uintptr_t asize = size > 32767 ? 32767 : size;
    ISC_STATUS_ARRAY status;
    if( api.isc_put_segment(status,&handle_,(short) asize,(char *) buf) != 0 )
      statement_->database_->exceptionHandler(
        newObjectV1C2<EDSQLStPutSegment>(status, __PRETTY_FUNCTION__)
      );
    buf = (const char *) buf + asize;
    size -= asize;
  }
  sqlind_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::setNull()
{
  if( next_ != NULL ) next_->setNull();
  sqlind_ = -1;
  cancelBlob();
  clear();
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::putSlice()
{
  if( sqlind_ >= 0 )
    statement_->arrayPutSlice(id_,arrayDesc_,data_,dataSize_);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::createBlob()
{
  statement_->createBlob(handle_,id_,0,NULL);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::closeBlob()
{
  statement_->closeBlob(handle_);
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam & DSQLParam::cancelBlob()
{
  statement_->cancelBlob(handle_);
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLParams::~DSQLParams()
{
}
//---------------------------------------------------------------------------
DSQLParams::DSQLParams() : statement_(NULL), paramsAutoDrop_(params_)
{
}
//---------------------------------------------------------------------------
DSQLParams & DSQLParams::bind()
{
  for( intptr_t j, i = indexToParam_.count() - 1; i >= 0; i-- ){
    DSQLParam * param = indexToParam_[i];
    XSQLVAR & v = sqlda_.sqlda()->sqlvar[i];
    switch( v.sqltype & ~1 ){
      case SQL_VARYING     :
      case SQL_TEXT        :
        v.sqltype = (int16_t) (SQL_TEXT | (v.sqltype & 1));
        v.sqllen = (int16_t) param->text_.size();
        v.sqldata = param->text_.c_str();
        break;
      case SQL_SHORT       :
      case SQL_LONG        :
        v.sqldata = (char *) &param->bigInt_;
        break;
      case SQL_FLOAT       :
        v.sqldata = (char *) &param->float_;
        for( j = v.sqlscale; j < 0; j++ ) param->float_ *= 10;
        break;
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
        v.sqldata = (char *) &param->double_;
        for( j = v.sqlscale; j < 0; j++ ) param->double_ *= 10;
        break;
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
        v.sqldata = (char *) &param->timeStamp_;
        break;
      case SQL_QUAD        :
      case SQL_INT64       :
        v.sqldata = (char *) &param->bigInt_;
        break;
      case SQL_BLOB :
        v.sqldata = (char *) &param->id_;
        break;
      case SQL_ARRAY :
        v.sqldata = (char *) &param->id_;
        param->putSlice();
    }
    v.sqlind = &param->sqlind_;
    param->sqltype_ = v.sqltype;
    param->sqlscale_ = (char) v.sqlscale;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLParam * DSQLParams::checkParamIndex(uintptr_t i)
{
  if( i >= indexToParam_.count() )
    newObjectV1C2<EDSQLStInvalidParamIndex>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return indexToParam_[i];
}
//---------------------------------------------------------------------------
DSQLParam * DSQLParams::checkParamName(const utf8::String & paramName)
{
  DSQLParam * param = params_.find(DSQLParam(NULL,paramName));
  if( param == NULL )
    newObjectV1C2<EDSQLStInvalidParamName>((const ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return param;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
