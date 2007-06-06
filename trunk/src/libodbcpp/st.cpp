/*-
 * Copyright 2007 Guram Dukashvili
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
#endif
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLStatement::~DSQLStatement()
{
  params_.params_.drop();
  detach();
}
//---------------------------------------------------------------------------
DSQLStatement::DSQLStatement() :
  database_(NULL),
  handle_(NULL),
  sqlTextChanged_(false),
  prepared_(false),
  executed_(false)
{
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::attach(Database & database)
{
  if( attached() )
    newObjectV1C2<EClientServer>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  database.statements_.insToTail(*this);
  database_ = &database;
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::detach()
{
  if( attached() ){
    freeHandle();
    database_->statements_.remove(*this);
    database_ = NULL;
    prepared_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::allocateHandle()
{
  if( attached() && handle_ == NULL ){
    SQLRETURN r = api.SQLAllocHandle(SQL_HANDLE_STMT,database_->handle_,&handle_);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_ENV,database_->handle_));
    r = api.SQLSetStmtAttr(handle_,SQL_ATTR_ASYNC_ENABLE,(SQLPOINTER) SQL_ASYNC_ENABLE_OFF,0);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ){
      utf8::String state;
      ksys::AutoPtr<EClientServer> e(database_->exception(SQL_HANDLE_STMT,handle_,&state));
      if( state.strcmp("HYC00") != 0 ) database_->exceptionHandler(e.ptr(NULL));
    }
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::freeHandle()
{
  if( handle_ != NULL ){
    api.SQLFreeHandle(SQL_HANDLE_STMT,handle_);
    handle_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLStatement::compileSQLParameters()
{
  params_.params_.drop();
  params_.indexToParam_.clear();
  utf8::String text(sqlText_.unique());
  utf8::String::Iterator i(text);
  while( !i.eos() ){
    uintptr_t c = i.getChar();
    if( c == ':' ){
      utf8::String::Iterator i2(i);
      while( i2.next() && ((c = i2.getChar()) == '_' || (utf8::getC1Type(c) & (C1_ALPHA | C1_DIGIT)) != 0) );
      if( i2 - i > 1 && !(i + 1).isDigit() ){
        DSQLParam * p;
        ksys::AutoPtr<DSQLParam> param(p = newObjectV1<DSQLParam>(this));
        param->name_ = utf8::String(i + 1,i2);
        params_.params_.insert(param,false,false,&p);
        p->index_ = params_.indexToParam_.count();
        params_.indexToParam_.add(p);
        param.ptr(NULL);
        text.replace(i,i2,"?");
      }
    }
    i.next();
  }
  return text;
}
//---------------------------------------------------------------------------
int16_t DSQLParam::sqlType(void * & data,SQL_INTEGER_T & len)
{
  len = 0;
  switch( type_ ){
    case ksys::mtNull   : data = (void *) SQL_NULL_DATA; return SQL_C_DEFAULT;
    case ksys::mtInt    : data = &int_; return SQL_C_SBIGINT;
    case ksys::mtFloat  : data = &float_; return SQL_C_DOUBLE;
    case ksys::mtTime   : data = &time_; return SQL_C_TYPE_TIMESTAMP;
    case ksys::mtCStr   :
    case ksys::mtWStr   :
    case ksys::mtStr    :
    case ksys::mtString : data = string_.ptr(); len = SQL_NTS; return SQL_C_WCHAR;
    case ksys::mtBinary : data = stream_.raw(); len = (long) stream_.count(); return SQL_C_BINARY;
  }
  return SQL_C_DEFAULT;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::prepare()
{
  allocateHandle();
  if( sqlTextChanged_ || !prepared_ ){
    utf8::String sql(compileSQLParameters());
    SQLRETURN r;
    r = api.SQLPrepareW(handle_,sql.getUNICODEString(),SQL_NTS);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));
/*    r = api.SQLNumResultCols(handle_,&j);
      database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));*/
    SQLSMALLINT j = -1;
    r = api.SQLNumParams(handle_,&j);
      database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));
    if( (uintptr_t) j != params_.indexToParam_.count() )
      database_->exceptionHandler(newObjectV1C2<EClientServer>(EINVAL,"ODBC SQLNumParams failed " + utf8::String(__PRETTY_FUNCTION__)));
    for( SQLUSMALLINT i = 0; i < params_.indexToParam_.count(); i++ ){
      SQLSMALLINT dataType, dataCType, decimalDigits, nullable;
      SQLUINTEGER paramSize;
      r = api.SQLDescribeParam(handle_,i + 1,&dataType,&paramSize,&decimalDigits,&nullable);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));
      SQLPOINTER data;
      SQLINTEGER len;
      dataCType = params_.indexToParam_[uintptr_t(i)]->sqlType(data,len);
      r = api.SQLBindParameter(
        handle_,
        i + 1,
        SQL_PARAM_INPUT,
        dataCType,
        dataType,
        paramSize,
        decimalDigits,
        data,
        len,
        NULL
      );
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));
    }
    sqlTextChanged_ = false;
    prepared_ = true;
  }
  else if( executed_ ){
//    values_.clear().valuesIndex_.clear();
    executed_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::execute()
{
  database_->transaction_->start();
  prepare();
  SQLRETURN r = api.SQLExecute(handle_);
  if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
    database_->exceptionHandler(database_->exception(SQL_HANDLE_STMT,handle_));
  executed_ = true;
//  if( database_->transaction_->startCount_ == 1 )
//    values_.fetchAll();
  database_->transaction_->commit();
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// properties ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::sqlText(const utf8::String & sqlText)
{
  sqlText_ = sqlText;
  sqlTextChanged_ = true;
  prepared_ = false;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
