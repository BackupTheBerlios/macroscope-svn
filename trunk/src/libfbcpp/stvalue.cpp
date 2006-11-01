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
ksys::Mutant DSQLValue::getMutant()
{
  if( sqlind_ >= 0 )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
utf8::String DSQLValue::getString()
{
  if( sqlind_ >= 0 )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return utf8::String();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLValueText::DSQLValueText()
{
}
//---------------------------------------------------------------------------
DSQLValueText::~DSQLValueText()
{
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLValueText::getMutant()
{
  if( sqlind_ < 0 ) return ksys::Mutant();
  return text_;
}
//---------------------------------------------------------------------------
utf8::String DSQLValueText::getString()
{
  return text_;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLValueScalar::DSQLValueScalar()
{
}
//---------------------------------------------------------------------------
DSQLValueScalar::~DSQLValueScalar()
{
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLValueScalar::getMutant()
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
        v = (int16_t) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v /= 10;
        return v;
      }
      return (short) bigInt_;
    case SQL_LONG        :
      if( sqlscale_ != 0 ){
        v = (int32_t) bigInt_;
        for( intptr_t j = sqlscale_; j < 0; j++ )
          v /= 10;
        return v;
      }
      return (int32_t) bigInt_;
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
  newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
utf8::String DSQLValueScalar::getString()
{
#if HAVE_LONG_DOUBLE
  long
  #endif
  double v;
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
          return utf8::float2Str(v);
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
        newObject<EDSQLStInvalidParamValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
    }
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLValueArray::DSQLValueArray(DSQLStatement & statement)
  : statement_(&statement),
    data_(NULL)
{
}
//---------------------------------------------------------------------------
DSQLValueArray::~DSQLValueArray()
{
  ksys::xfree(data_);
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValueArray::clear()
{
  ksys::xfree(data_);
  data_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValueArray::checkDim(bool inRange)
{
  if( !inRange )
    newObject<EDSQLStInvalidArrayDim>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *this;
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValueArray::checkData()
{
  if( data_ == NULL ){
    intptr_t  i;
    dataSize_ = desc_.array_desc_length;
    if( desc_.array_desc_dtype == blr_varying )
      dataSize_ += 2;
    elementSize_ = dataSize_;
    for( i = desc_.array_desc_dimensions - 1; i >= 0; i-- ){
      dimElements_[i] = desc_.array_desc_bounds[i].array_bound_upper - desc_.array_desc_bounds[i].array_bound_lower + 1;
      dataSize_ *= (ISC_LONG) dimElements_[i];
    }
    memmove(dimElements_, dimElements_ + 1, sizeof(dimElements_) - sizeof(dimElements_[0]));
    dimElements_[desc_.array_desc_dimensions - 1] = 1;
    for( i = desc_.array_desc_dimensions - 1; i > 0; i-- ){
      dimElements_[i - 1] *= dimElements_[i];
    }
    ksys::xmalloc(data_, dataSize_);
  }
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLValueArray::getMutantFromArray(uintptr_t absIndex)
{
  if( sqlind_ < 0 )
    return ksys::Mutant();
  checkData();
  uintptr_t elementOffset = elementSize_ * absIndex;
  if( elementOffset < (uintptr_t) dataSize_ ){
    char *  data  = (char *) data_ + elementOffset;
    switch( desc_.array_desc_dtype ){
      case blr_varying   :
        return utf8::plane(data, *(int16_t *) (data + elementSize_ - 2));
      case blr_text      :
        return utf8::plane(data, elementSize_);
      case blr_short     :
        return *(int16_t *) data;
      case blr_long      :
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
        return *(int64_t *) data;
    }
  }
  newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  exit(ENOSYS);
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValueArray::getSlice()
{
  if( sqlind_ >= 0 )
    statement_->arrayGetSlice(id_, desc_, data_, dataSize_);
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLValueBlob::DSQLValueBlob(DSQLStatement & statement)
  : statement_(&statement)
{
}
//---------------------------------------------------------------------------
DSQLValueBlob::~DSQLValueBlob()
{
}
//---------------------------------------------------------------------------
utf8::String DSQLValueBlob::getString()
{
  if( sqlind_ >= 0 ){
    closeBlob().openBlob();
    intptr_t              r, l  = 0;
    ksys::AutoPtr< char>  aref;
    aref.alloc(desc_.blob_desc_segment_size + 1);
    try{
      for( ; ; ){
        r = readBuffer(aref.ptr() + l, desc_.blob_desc_segment_size);
        if( r <= 0 )
          break;
        l += r;
        aref.realloc(l + desc_.blob_desc_segment_size + 1);
      }
    }
    catch( ksys::ExceptionSP & ){
      closeBlob();
      throw;
    }
    closeBlob();
    aref.realloc(l + 1);
    return utf8::plane(aref, l);
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
DSQLValueBlob & DSQLValueBlob::openBlob()
{
  statement_->openBlob(handle_, id_, 0, NULL);
  return *this;
}
//---------------------------------------------------------------------------
DSQLValueBlob & DSQLValueBlob::closeBlob()
{
  statement_->closeBlob(handle_);
  return *this;
}
//---------------------------------------------------------------------------
intptr_t DSQLValueBlob::readBuffer(void * buf, uintptr_t size)
{
  ISC_STATUS_ARRAY  status;
  uint16_t          segLen, len;
  intptr_t          blobStat, r = -1;
  if( handle_ != 0 ){
    r = 0;
    while( size > 0 ){
      len = (uint16_t) size;
      if( size > (uintptr_t) desc_.blob_desc_segment_size )
        len = desc_.blob_desc_segment_size;
      blobStat = api.isc_get_segment(status, &handle_, &segLen, len, (char *) buf);
      buf = (char *) buf + segLen;
      size -= segLen;
      r += segLen;
      if( blobStat != 0 ){
        if( status[1] == isc_segment )
          continue;
        if( status[1] == isc_segstr_eof )
          break;
        statement_->database_->exceptionHandler(
          newObject<EDSQLStGetSegment>(status, __PRETTY_FUNCTION__)
        );
      }
    }
  }
  return r;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLValues::DSQLValues(DSQLStatement & statement)
  : statement_(&statement),
    row_(~uintptr_t(0) >> 1)
{
  valuesIndex_.caseSensitive(false).ownsObjects(false);
}
//---------------------------------------------------------------------------
DSQLValues::~DSQLValues()
{
}
//---------------------------------------------------------------------------
DSQLValueBlob & DSQLValues::asBlob(uintptr_t i)
{
  DSQLValueBlob * blob  = dynamic_cast< DSQLValueBlob *>(&rows_[row_][checkValueIndex(i)]);
  if( blob == NULL )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *blob;
}
//---------------------------------------------------------------------------
DSQLValueBlob & DSQLValues::asBlob(const utf8::String & valueName)
{
  DSQLValueBlob * blob  = dynamic_cast< DSQLValueBlob *>(&rows_[row_][checkValueIndex(valuesIndex_.indexOfKey(valueName))]);
  if( blob == NULL )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *blob;
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValues::asArray(uintptr_t i)
{
  DSQLValueArray *  array = dynamic_cast< DSQLValueArray *>(&rows_[row_][checkValueIndex(i)]);
  if( array == NULL )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *array;
}
//---------------------------------------------------------------------------
DSQLValueArray & DSQLValues::asArray(const utf8::String & valueName)
{
  DSQLValueArray *  array = dynamic_cast< DSQLValueArray *>(&rows_[row_][checkValueIndex(valuesIndex_.indexOfKey(valueName))]);
  if( array == NULL )
    newObject<EDSQLStInvalidValue>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  return *array;
}
//---------------------------------------------------------------------------
ksys::Vector< DSQLValue> * DSQLValues::bind()
{
  ksys::AutoPtr<ksys::Vector<DSQLValue> > row(newObject<ksys::Vector<DSQLValue> >());
  union {
      DSQLValue *       value;
      DSQLValueText *   valueText;
      DSQLValueScalar * valueScalar;
      DSQLValueArray *  valueArray;
      DSQLValueBlob *   valueBlob;
  };
  uintptr_t i, j;
  for( i = 0; i < sqlda_.count(); i++ ){
    XSQLVAR & v = sqlda_.sqlda()->sqlvar[i];
    switch( v.sqltype & ~1 ){
      case SQL_VARYING     :
        row->safeAdd(valueText = newObject<DSQLValueText>());
        v.sqldata = valueText->text_.resize(v.sqllen + 2).c_str();
        break;
      case SQL_TEXT        :
        row->safeAdd(valueText = newObject<DSQLValueText>());
        v.sqldata = valueText->text_.resize(v.sqllen).c_str();
        break;
      case SQL_SHORT       :
      case SQL_LONG        :
      case SQL_FLOAT       :
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
      case SQL_QUAD        :
      case SQL_INT64       :
        row->safeAdd(valueScalar = newObject<DSQLValueScalar>());
        v.sqldata = (char *) &valueScalar->timeStamp_;
        valueScalar->sqlscale_ = (char) v.sqlscale;
        break;
      case SQL_BLOB        :
        row->safeAdd(valueBlob = newObjectV<DSQLValueBlob>(*statement_));
        valueBlob->handle_ = 0;
        statement_->blobLookupDesc(v.relname, v.sqlname, valueBlob->desc_, NULL);
        v.sqldata = (char *) &valueBlob->id_;
        break;
      case SQL_ARRAY       :
        row->safeAdd(valueArray = newObjectV<DSQLValueArray>(*statement_));
        valueArray->id_.gds_quad_low = 0;
        valueArray->id_.gds_quad_high = 0;
        statement_->arrayLookupBounds(v.relname, v.sqlname, valueArray->desc_);
        v.sqldata = (char *) &valueArray->id_;
        break;
    }
    value->sqlind_ = v.sqltype & 1 ? -1 : 0;
    v.sqlind = &value->sqlind_;
    value->sqltype_ = v.sqltype;
  }
  if( valuesIndex_.count() != row->count() ){
    for( i = 0; i < row->count(); i++ ){
      XSQLVAR &     v   = sqlda_.sqlda()->sqlvar[i];
      utf8::String  key (utf8::plane(v.sqlname, v.sqlname_length));
      utf8::String testKey(key);
      for( j = 1; j < ~uintptr_t(0); j++ ){
        if( valuesIndex_.objectOfKey(testKey) == NULL )
          break;
        testKey = key + "_" + utf8::int2Str((intmax_t) j);
      }
      valuesIndex_.add(row.ptr()->ptr()[i], testKey);
    }
  }
  return row.ptr(NULL);
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::fillRow(ksys::Vector< DSQLValue> * row)
{
  union {
      DSQLValue *       value;
      DSQLValueText *   valueText;
      //    DSQLValueScalar * ValueScalar;
      DSQLValueArray *  valueArray;
      //    DSQLValueBlob * ValueBlob;
  };
  for( intptr_t strLen, i = row->count() - 1; i >= 0; i-- ){
    XSQLVAR & v = sqlda_.sqlda()->sqlvar[i];
    if( v.sqlind < 0 )
      continue;
    value = &(*row)[i];
    switch( v.sqltype & ~1 ){
      case SQL_VARYING     :
        strLen = api.isc_vax_integer(valueText->text_.c_str(), 1);
        valueText->text_ = utf8::plane(valueText->text_.c_str() + 2, strLen);
        break;
      case SQL_TEXT        :
        break;
      case SQL_SHORT       :
      case SQL_LONG        :
      case SQL_FLOAT       :
      case SQL_DOUBLE      :
      case SQL_D_FLOAT     :
      case SQL_TYPE_TIME   :
      case SQL_TYPE_DATE   :
      case SQL_TIMESTAMP   :
      case SQL_QUAD        :
      case SQL_INT64       :
        break;
      case SQL_BLOB        :
        break;
      case SQL_ARRAY       :
        valueArray->checkData();
        valueArray->getSlice();
        break;
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLValues::fetch()
{
  ISC_STATUS                                code;
  ISC_STATUS_ARRAY                          status;
  ksys::AutoPtr< ksys::Vector< DSQLValue> > row (bind());
  code = statement_->fetch(status);
  if( code != 100 && code != isc_req_sync ){
    fillRow(row.ptr());
    rows_.add(row.ptr());
    if( row_ < 0 ) row_ = 0;
    row.ptr(NULL);
  }
  return code != 100 && code != isc_req_sync;
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::fetchAll()
{
  while( fetch() );
  return *this;
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::selectRow(uintptr_t i)
{
  if( i >= rows_.count() )
    newObject<EDSQLStInvalidRowIndex>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  row_ = i;
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t DSQLValues::checkValueIndex(uintptr_t i)
{
  if( row_ < 0 )
    newObject<EDSQLStInvalidRowIndex>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__)->throwSP();
  if( i >= valuesIndex_.count() )
    newObject<EDSQLStInvalidValueIndex>(
      (ISC_STATUS *) NULL, utf8::String(__PRETTY_FUNCTION__) + ", index = " + utf8::int2Str((intmax_t) i)
    )->throwSP();
  return i;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
