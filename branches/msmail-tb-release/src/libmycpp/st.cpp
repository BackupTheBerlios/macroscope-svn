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
utf8::String time2Str(int64_t t)
{
  struct tm     tv  = ksys::time2tm(t);
  utf8::String  s;
  s.resize(19);
  sprintf(s.c_str(), "%04d-%02d-%02d %02d:%02d:%02d", tv.tm_year + 1900, tv.tm_mon + 1, tv.tm_mday, tv.tm_hour, tv.tm_min, tv.tm_sec);
  return s;
}
//---------------------------------------------------------------------------
int64_t str2Time(const char * s)
{
  struct tm tv;
  memset(&tv, 0, sizeof(tv));
  sscanf(s, "%04d-%02d-%02d %02d:%02d:%02d", &tv.tm_year, &tv.tm_mon, &tv.tm_mday, &tv.tm_hour, &tv.tm_min, &tv.tm_sec);
  tv.tm_year -= 1900;
  tv.tm_mon--;
  return ksys::tm2Time(tv);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLStatement::DSQLStatement()
  : handle_(NULL),
    database_(NULL),
    sqlTextChanged_(false),
    prepared_(false),
    executed_(false),
    storeResults_(false),
#if _MSC_VER
#pragma warning(disable:4355)
#endif
  params_(*this),
    values_(*this)
#if _MSC_VER
#pragma warning(default:4355)
#endif

{
}
//---------------------------------------------------------------------------
DSQLStatement::~DSQLStatement()
{
  detach();
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::attach(Database & database)
{
  if( attached() )
    throw ksys::ExceptionSP(new EDSQLStAttached(EINVAL, __PRETTY_FUNCTION__));
  database.dsqlStatements_.add(this, utf8::ptr2Str(this));
  database_ = &database;
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::detach()
{
  if( attached() ){
    values_.freeRes();
    free();
    database_->dsqlStatements_.removeByObject(this);
    database_ = NULL;
    prepared_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::allocate()
{
  if( !attached() )
    throw ksys::ExceptionSP(new EDSQLStNotAttached(EINVAL, __PRETTY_FUNCTION__));
  if( !allocated() ){
    handle_ = api.mysql_stmt_init(database_->handle_);
    if( handle_ == NULL )
      database_->exceptionHandler(new EDSQLStAllocate(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::free()
{
  if( allocated() ){
    if( api.mysql_stmt_close(handle_) != 0 )
      database_->exceptionHandler(new EDSQLStFree(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
    handle_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool DSQLStatement::isSQLTextDDL() const
{
  utf8::String  upperText (sqlText_.trimLeft().upper());
  return
    upperText.strncasecmp("CREATE", 6) == 0 ||
    upperText.strncasecmp("ALTER", 5) == 0
  ;
}
//---------------------------------------------------------------------------
utf8::String DSQLStatement::compileSQLParameters()
{
  params_.indexToParam_.clear();
  params_.resetChanged();
  utf8::String text(sqlText_);
  if( !isSQLTextDDL() ){
    text = text.unique();
    utf8::String::Iterator i(text);
    while( !i.eof() ){
      uintptr_t c = i.getChar();
      if( c == ':' ){
        utf8::String::Iterator i2(i);
        while( i2.next() && ((c = i2.getChar()) == '_' || (utf8::getC1Type(c) & (C1_ALPHA | C1_DIGIT)) != 0) );
        if( i2 - i > 1 && !(i + 1).isDigit() ){
          params_.indexToParam_.add(params_.add(utf8::String(i + 1, i2)));
          text.replace(i, i2, "?");
        }
      }
      i.next();
    }
  }
  params_.removeUnchanged();
  return text;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::prepare()
{
  allocate();
  if( sqlTextChanged_ || !prepared_ ){
    utf8::String  sql (compileSQLParameters());
    if( api.mysql_stmt_prepare(handle_, sql.c_str(), (unsigned long) sql.size()) != 0 ){
      if( api.mysql_errno(database_->handle_) != ER_UNSUPPORTED_PS )
        database_->exceptionHandler(new EDSQLStPrepare(
          api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
    }
    params_.bind_.resize(api.mysql_stmt_param_count(handle_));
    values_.bind_.resize(api.mysql_stmt_field_count(handle_));
    values_.clear().valuesIndex_.clear();
    sqlTextChanged_ = false;
    prepared_ = true;
  }
  else if( executed_ ){
    values_.clear();
    executed_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::execute()
{
  database_->transaction_->start();
  values_.freeRes();
  prepare();
  if( params_.bind_.count() > 0 ){
    params_.bind();
    if( api.mysql_stmt_bind_param(handle_, params_.bind_.bind()) != 0 ){
      database_->exceptionHandler(new EDSQLStBindParam(
          api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
    }
  }
  if( params_.bind_.count() == 0 && values_.bind_.count() == 0 ){
    if( api.mysql_query(database_->handle_, compileSQLParameters().c_str()) != 0 )
      database_->exceptionHandler(new EDSQLStExecute(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
  }
  else{
    if( api.mysql_stmt_execute(handle_) != 0 ){
      database_->exceptionHandler(new EDSQLStExecute(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
    }
  }
  executed_ = true;
  if( values_.bind_.count() > 0 ){
    values_.lengths_.resize(values_.bind_.count());
    values_.res_ = api.mysql_stmt_result_metadata(handle_);
    if( api.mysql_errno(database_->handle_) != 0 )
      database_->exceptionHandler(new EDSQLStResultMetadata(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
    values_.fields_ = api.mysql_fetch_fields(values_.res_);
  }
  /* Now buffer all results to client */
  if( storeResults_ )
    if( api.mysql_stmt_store_result(handle_) != 0 )
      database_->exceptionHandler(new EDSQLStStoreResult(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
  if( database_->transaction_->startCount_ == 1 && values_.bind_.count() > 0 )
    values_.fetchAll();
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
} // namespace fbcpp
//---------------------------------------------------------------------------
