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
#include <adicpp/mycpp.h>
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLRow * DSQLValues::bind()
{
  ksys::AutoPtr<DSQLRow> row(new DSQLRow);
  row->index_.resize(bind_.count());
  row->isNulls_.resize(bind_.count());
  uintptr_t i;
  memset(bind_.bind(),0,sizeof(MYSQL_BIND) * bind_.count());
  for( i = 0; i < bind_.count(); i++ ){
    row->index_[i] = row->rowSize_;
    MYSQL_BIND & rbind = bind_.bind()[i];
    MYSQL_FIELD & field = fields_[i];
    switch( field.type ){
      case MYSQL_TYPE_DECIMAL :
        goto l1;
      case MYSQL_TYPE_TINY    :
        rbind.buffer_type = MYSQL_TYPE_TINY;
        row->rowSize_ += rbind.buffer_length = sizeof(int8_t);
        break;
      case MYSQL_TYPE_SHORT   :
        rbind.buffer_type = MYSQL_TYPE_SHORT;
        row->rowSize_ += rbind.buffer_length = sizeof(int16_t);
        break;
      case MYSQL_TYPE_LONG    :
        rbind.buffer_type = MYSQL_TYPE_LONG;
        row->rowSize_ += rbind.buffer_length = sizeof(int32_t);
        break;
      case MYSQL_TYPE_FLOAT   :
        rbind.buffer_type = MYSQL_TYPE_FLOAT;
        row->rowSize_ += rbind.buffer_length = sizeof(float);
        break;
      case MYSQL_TYPE_DOUBLE  :
        rbind.buffer_type = MYSQL_TYPE_DOUBLE;
        row->rowSize_ += rbind.buffer_length = sizeof(double);
        break;
      case MYSQL_TYPE_NULL    :
      case MYSQL_TYPE_TIMESTAMP :
        goto l1;
      case MYSQL_TYPE_LONGLONG :
        rbind.buffer_type = MYSQL_TYPE_LONGLONG;
        row->rowSize_ += rbind.buffer_length = sizeof(int64_t);
        break;
      case MYSQL_TYPE_INT24 :
        rbind.buffer_type = MYSQL_TYPE_INT24;
        row->rowSize_ += rbind.buffer_length = sizeof(int32_t) - 1;
        break;
      case MYSQL_TYPE_DATE :
      case MYSQL_TYPE_TIME :
      case MYSQL_TYPE_DATETIME :
      case MYSQL_TYPE_YEAR :
      case MYSQL_TYPE_NEWDATE :
        goto l1;
      case MYSQL_TYPE_ENUM :
        rbind.buffer_type = MYSQL_TYPE_ENUM;
        row->rowSize_ += rbind.buffer_length = field.length;
        break;
      case MYSQL_TYPE_SET :
        rbind.buffer_type = MYSQL_TYPE_SET;
        row->rowSize_ += rbind.buffer_length = field.length;
        break;
      case MYSQL_TYPE_TINY_BLOB :
      case MYSQL_TYPE_MEDIUM_BLOB :
      case MYSQL_TYPE_LONG_BLOB :
      case MYSQL_TYPE_BLOB :
        rbind.buffer_type = field.type;
        row->rowSize_ += rbind.buffer_length = field.length;
        break;
      case MYSQL_TYPE_VAR_STRING :
      case MYSQL_TYPE_STRING :
l1:     rbind.buffer_type = MYSQL_TYPE_STRING;
        row->rowSize_ += rbind.buffer_length = field.length + 1;
        break;
      case MYSQL_TYPE_GEOMETRY : // not supported in MySQL 4.1
        rbind.buffer_type = field.type;
        row->rowSize_ += rbind.buffer_length = field.length;
        break;
      default : ;
    }
    rbind.is_null = &row->isNulls_[i];
    rbind.length = lengths_.ptr() + i;
  }
  row->raw_.realloc(row->rowSize_);
  for( i = 0; i < bind_.count(); i++ )
    bind_.bind()[i].buffer = row->raw_.ptr() + row->index_[i];
  if( valuesIndex_.count() != bind_.count() ){
    for( i = 0; i < bind_.count(); i++ ){
      utf8::String key(utf8::plane(fields_[i].name,fields_[i].name_length));
      utf8::String::Iterator it(key.strstr(utf8::string("(")));
      if( it.position() >= 0 ) key = utf8::string(utf8::String::Iterator(key),it);
      utf8::String testKey(key);
      for( uintptr_t j = 1; j < ~uintptr_t(0); j++ ){
        if( valuesIndex_.objectOfKey(testKey) == NULL ) break;
        testKey = key + "_" + utf8::int2Str((intmax_t) j);
      }
      valuesIndex_.add(&row->index_[i],testKey);
    }
  }
  return row.ptr(NULL);
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::fillRowHelper(struct fillRowVars & v,uintptr_t ds)
{
  v.k = v.rbind->buffer_length - ds;
  memcpy(v.pc + ds,v.pc + v.rbind->buffer_length,
    v.row->rowSize_ - v.row->index_[v.i] - v.rbind->buffer_length
  );
  for( v.j = bind_.count() - 1; v.j > v.i; v.j-- )
    if( v.row->index_[v.j] >= 0 ) v.row->index_[v.j] -= (int32_t) v.k;
  v.row->rowSize_ -= (int32_t) v.k;
  return *this;
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::fillRow(DSQLRow * row)
{
  struct fillRowVars v;
  v.row = row;
  uintptr_t rowSize = v.row->rowSize_/*, a*/;
  for( v.i = bind_.count() - 1; v.i >= 0; v.i-- ){
    v.rbind = &bind_.bind()[v.i];
    v.pc = (char *) (row->raw_.ptr() + v.row->index_[v.i]);
    if( v.row->isNulls_[v.i] ){
      fillRowHelper(v,0);
      v.row->index_[v.i] = ~uintptr_t(0);
    }
    else {
      MYSQL_FIELD & field = fields_[v.i];
      switch( field.type ){
        case MYSQL_TYPE_DECIMAL :
          goto l2;
        case MYSQL_TYPE_TINY    :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.pc = (int8_t) utf8::str2Int(v.pc);
//          fillRowHelper(v,sizeof(int8_t));
//          break;
        case MYSQL_TYPE_SHORT   :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.ps = (int16_t) utf8::str2Int(v.pc);
//          fillRowHelper(v,sizeof(int16_t));
//          break;
        case MYSQL_TYPE_LONG    :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.pl = (int32_t) utf8::str2Int(v.pc);
//          fillRowHelper(v,sizeof(int32_t));
//          break;
        case MYSQL_TYPE_FLOAT   :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.pf = utf8::str2Float(v.pc);
//          fillRowHelper(v,sizeof(float));
//          break;
        case MYSQL_TYPE_DOUBLE  :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.pd = utf8::str2Float(v.pc);
//          fillRowHelper(v,sizeof(double));
          break;
        case MYSQL_TYPE_NULL    :
          break;
        case MYSQL_TYPE_TIMESTAMP :
          goto l1;
        case MYSQL_TYPE_LONGLONG :
//          v.pc[lengths_[v.i]] = '\0';
//          *v.pll = utf8::str2Int(v.pc);
//          fillRowHelper(v,sizeof(int64_t));
          break;
        case MYSQL_TYPE_INT24 :
//          v.pc[lengths_[v.i]] = '\0';
//          a = utf8::str2Int(v.pc);
//          *v.pus = (uint16_t) a;
//          v.pc[2] = char(a >> 16);
//          fillRowHelper(v,3);
          break;
        case MYSQL_TYPE_DATE :
        case MYSQL_TYPE_TIME :
        case MYSQL_TYPE_DATETIME :
        case MYSQL_TYPE_YEAR :
        case MYSQL_TYPE_NEWDATE :
l1:       v.pc[lengths_[v.i]] = '\0';
          *v.pll = str2Time(v.pc);
          fillRowHelper(v,sizeof(int64_t));
          break;
        case MYSQL_TYPE_ENUM :
        case MYSQL_TYPE_SET :
          break;
        case MYSQL_TYPE_TINY_BLOB :
        case MYSQL_TYPE_MEDIUM_BLOB :
        case MYSQL_TYPE_LONG_BLOB :
        case MYSQL_TYPE_BLOB :
          fillRowHelper(v,lengths_[v.i]);
          break;
        case MYSQL_TYPE_VAR_STRING :
        case MYSQL_TYPE_STRING :
l2:       fillRowHelper(v,lengths_[v.i] + 1);
          v.pc[lengths_[v.i]] = '\0';
          break;
        case MYSQL_TYPE_GEOMETRY : // not supported in MySQL 4.1
          break;
        default : ;
      }
    }
  }
  if( rowSize != v.row->rowSize_ ) v.row->raw_.realloc(v.row->rowSize_);
  v.row->isNulls_.clear();
  return *this;
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::freeRes()
{
  if( res_ != NULL ){
    api.mysql_free_result(res_);
    if( api.mysql_stmt_free_result(statement_.handle_) != 0 )
      statement_.database_->exceptionHandler(new EDSQLStFreeResult(
        api.mysql_errno(statement_.database_->handle_),
        utf8::string(api.mysql_error(statement_.database_->handle_))
      ));
    res_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLValues::fetch()
{
  long code;
  ksys::AutoPtr<DSQLRow> row(bind());
  if( api.mysql_stmt_bind_result(statement_.handle_,bind_.bind()) != 0 )
    statement_.database_->exceptionHandler(new EDSQLStBindResult(
      api.mysql_errno(statement_.database_->handle_),
      utf8::string(api.mysql_error(statement_.database_->handle_))
    ));
  code = api.mysql_stmt_fetch(statement_.handle_);
  if( code == 1 )
    statement_.database_->exceptionHandler(new EDSQLStFetch(
      api.mysql_errno(statement_.database_->handle_),
      utf8::string(api.mysql_error(statement_.database_->handle_))
    ));
  if( code != MYSQL_NO_DATA ){
    fillRow(row);
    rows_.add(row.ptr());
    if( row_ < 0 ) row_ = 0;
    row.ptr(NULL);
  }
  return code != MYSQL_NO_DATA;
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::fetchAll()
{
  while( fetch() );
  return *this;
}
//---------------------------------------------------------------------------
ksys::Mutant DSQLValues::asMutant(uintptr_t i)
{
  DSQLRow & row = rows_[row_];
  if( row.index_[i = checkValueIndex(i)] < 0 ) return ksys::Mutant();
  union {
    char * pc;
    int16_t * ps;
    uint16_t * pus;
    int32_t * pl;
    int64_t * pll;
    float * pf;
    double * pd;
  };
  pc = (char *) (row.raw_.ptr() + row.index_[i]);
  switch( fields_[i].type ){
    case MYSQL_TYPE_DECIMAL     : return ksys::Mutant(pc,0);
    case MYSQL_TYPE_TINY        : return *pc;
    case MYSQL_TYPE_SHORT       : return *ps;
    case MYSQL_TYPE_LONG        : return *pl;
    case MYSQL_TYPE_FLOAT       : return *pf;
    case MYSQL_TYPE_DOUBLE      : return *pd;
    case MYSQL_TYPE_NULL        : return ksys::Mutant();
    case MYSQL_TYPE_TIMESTAMP   : goto l1;
    case MYSQL_TYPE_LONGLONG    : return *pll;
    case MYSQL_TYPE_INT24       : return *pus | (pc[2] << 16);
    case MYSQL_TYPE_DATE        :
    case MYSQL_TYPE_TIME        :
    case MYSQL_TYPE_DATETIME    :
    case MYSQL_TYPE_YEAR        :
    case MYSQL_TYPE_NEWDATE     :
l1:   return ksys::time2tm(*pll);
    case MYSQL_TYPE_ENUM        : return *ps;
    case MYSQL_TYPE_SET         : return *pll;
    case MYSQL_TYPE_TINY_BLOB   :
    case MYSQL_TYPE_MEDIUM_BLOB :
    case MYSQL_TYPE_LONG_BLOB   :
    case MYSQL_TYPE_BLOB        :
      return ksys::MemoryStream().readBuffer(pc,lengths_[i]).seek(0);
    case MYSQL_TYPE_VAR_STRING  :
    case MYSQL_TYPE_STRING      :
      return utf8::plane(pc);
    case MYSQL_TYPE_GEOMETRY :
      break;
    default : ;
  }
  throw ksys::ExceptionSP(new EDSQLStInvalidValue(-1,utf8::string(__PRETTY_FUNCTION__)));
}
//---------------------------------------------------------------------------
utf8::String DSQLValues::asString(uintptr_t i)
{
  DSQLRow & row = rows_[row_];
  if( row.index_[i = checkValueIndex(i)] < 0 ) return utf8::String();
  union {
    char * pc;
    short * ps;
    unsigned short * pus;
    int * pl;
    int64_t * pll;
    float * pf;
    double * pd;
  };
  pc = (char *) (row.raw_.ptr() + row.index_[i]);
  switch( fields_[i].type ){
    case MYSQL_TYPE_DECIMAL     : return ksys::Mutant(pc,0);
    case MYSQL_TYPE_TINY        : return utf8::int2Str(*pc);
    case MYSQL_TYPE_SHORT       : return utf8::int2Str(*ps);
    case MYSQL_TYPE_LONG        : return utf8::int2Str(*pl);
    case MYSQL_TYPE_FLOAT       : return utf8::float2Str(*pf);
    case MYSQL_TYPE_DOUBLE      : return utf8::float2Str(*pd);
    case MYSQL_TYPE_NULL        : return utf8::String();
    case MYSQL_TYPE_TIMESTAMP   : goto l1;
    case MYSQL_TYPE_LONGLONG    : return utf8::int2Str(*pll);
    case MYSQL_TYPE_INT24       : return utf8::int2Str(*pus | (pc[2] << 16));
    case MYSQL_TYPE_DATE        :
    case MYSQL_TYPE_TIME        :
    case MYSQL_TYPE_DATETIME    :
    case MYSQL_TYPE_YEAR        :
    case MYSQL_TYPE_NEWDATE     :
l1:   return utf8::time2Str(*pll);
    case MYSQL_TYPE_ENUM        :
    case MYSQL_TYPE_SET         : return utf8::int2Str(*pll);
    case MYSQL_TYPE_TINY_BLOB   :
    case MYSQL_TYPE_MEDIUM_BLOB :
    case MYSQL_TYPE_LONG_BLOB   :
    case MYSQL_TYPE_BLOB        :
      break;
    case MYSQL_TYPE_VAR_STRING  :
      return ksys::Mutant(pc,0);
    case MYSQL_TYPE_STRING      :
      return ksys::Mutant(pc,0);
    case MYSQL_TYPE_GEOMETRY :
      break;
    default : ;
  }
  throw ksys::ExceptionSP(new EDSQLStInvalidValue(-1,utf8::string(__PRETTY_FUNCTION__)));
}
//---------------------------------------------------------------------------
DSQLValues & DSQLValues::selectRow(uintptr_t i)
{
  if( i >= rows_.count() )
    throw ksys::ExceptionSP(new EDSQLStInvalidRowIndex(-1,utf8::string(__PRETTY_FUNCTION__)));
  row_ = i;
  return *this;
}
//---------------------------------------------------------------------------
uintptr_t DSQLValues::checkValueIndex(uintptr_t i)
{
  if( (uintptr_t) (intptr_t) row_ >= rows_.count() )
    throw ksys::ExceptionSP(new EDSQLStInvalidRowIndex(NULL,utf8::string(__PRETTY_FUNCTION__)));
  if( i >= valuesIndex_.count() )
    throw ksys::ExceptionSP(new EDSQLStInvalidValueIndex(-1,utf8::string(__PRETTY_FUNCTION__)));
  return i;
}
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------

