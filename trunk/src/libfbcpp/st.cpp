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
int64_t iscTimeStamp2Time(const ISC_TIMESTAMP & stamp)
{
  int64_t t = stamp.timestamp_date;
  t = (t << 16) + (t << 14) + (t << 12) + (t << 8) + (t << 7) + stamp.timestamp_time / 10000 - INT64_C(3506727600);
  t *= 1000000;
  t += (stamp.timestamp_time % 10000) * 100;
  return t;
}
//---------------------------------------------------------------------------
struct timeval iscTimeStamp2Timeval(const ISC_TIMESTAMP & stamp)
{
  struct timeval  tv;
  int64_t         t = stamp.timestamp_date;
  tv.tv_sec = long((t << 16) + (t << 14) + (t << 12) + (t << 8) + (t << 7) + stamp.timestamp_time / 10000 - INT64_C(3506727600));
  tv.tv_usec = (stamp.timestamp_time % 10000) * 100;
  return tv;
}
//---------------------------------------------------------------------------
struct tm iscTimeStamp2tm(const ISC_TIMESTAMP & stamp)
{
  time_t    t   = (stamp.timestamp_date << 16) + (stamp.timestamp_date << 14) + (stamp.timestamp_date << 12) + (stamp.timestamp_date << 8) + (stamp.timestamp_date << 7) + stamp.timestamp_time / 10000 - INT64_C(3506727600);
  struct tm ta  = *localtime(&t);
  ta.tm_isdst = 0;
  return ta;
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP time2IscTimeStamp(int64_t stamp)
{
  ISC_TIMESTAMP iscStamp;
  int64_t       t = stamp / 1000000 + INT64_C(3506727600);
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * 10000 + (stamp % 1000000) / 100);
  return iscStamp;
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP timeval2IscTimeStamp(const struct timeval & stamp)
{
  //  struct tm tmt(ksys::time2tm(ksys::timeval2Time(stamp))), tmt1;
  //  ISC_TIMESTAMP iscStamp1;
  //  api.isc_encode_timestamp(&tmt,&iscStamp1);
  //  struct tm tmt2;
  //  api.isc_decode_timestamp(&iscStamp1,&tmt2);

  ISC_TIMESTAMP iscStamp;
  int64_t       t = stamp.tv_sec + INT64_C(3506727600);
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * 10000 + stamp.tv_usec / 100);
  //  api.isc_decode_timestamp(&iscStamp,&tmt1);
  return iscStamp;
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP tm2IscTimeStamp(struct tm stamp)
{
  ISC_TIMESTAMP iscStamp;
  stamp.tm_isdst = 0;
  int64_t t = mktime(&stamp) + INT64_C(3506727600);
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * 10000);
  return iscStamp;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
XSQLDAHolder::XSQLDAHolder()
  : sqlda_(NULL)
{
#if _MSC_VER
#pragma warning(disable:4307)
#pragma warning(disable:4308)
#endif
  ksys::xmalloc(sqlda_, XSQLDA_LENGTH(0));
#if _MSC_VER
#pragma warning(default:4308)
#pragma warning(default:4307)
#endif
  sqlda_->sqln = 0;
  sqlda_->version = SQLDA_VERSION1;
}
//---------------------------------------------------------------------------
XSQLDAHolder::~XSQLDAHolder()
{
  ksys::xfree(sqlda_);
}
//---------------------------------------------------------------------------
XSQLDAHolder & XSQLDAHolder::resize(long n)
{
  if( n < 0 )
    n = 0;
  ksys::xrealloc(sqlda_, XSQLDA_LENGTH(n));
  sqlda_->sqln = (short) n;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
DSQLStatement::~DSQLStatement()
{
  detach();
}
//---------------------------------------------------------------------------
DSQLStatement::DSQLStatement()
  : handle_(0),
    database_(NULL),
    transaction_(NULL),
    sqlTextChanged_(false),
    prepared_(false),
    executed_(false),
#if _MSC_VER
#pragma warning(disable:4355)
#endif
  params_(*this),
    values_(*this),
#if _MSC_VER
#pragma warning(default:4355)
#endif
  stmtType(stmtUnknown)
{
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::attach(Database & database, Transaction & transaction)
{
  if( attached() )
    database.exceptionHandler(new EDSQLStAttached(NULL, __PRETTY_FUNCTION__));
  if( database.transactions_.itemOfObject(&transaction) == NULL )
    database.exceptionHandler(new EDSQLStNotAttached(NULL, __PRETTY_FUNCTION__));
  utf8::String  key (utf8::ptr2Str(this));
  database.dsqlStatements_.add(this, key);
  database_ = &database;
  try{
    transaction.dsqlStatements_.add(this, key);
  }
  catch( ksys::ExceptionSP & ){
    database.dsqlStatements_.removeByObject(this);
    database_ = NULL;
    throw;
  }
  transaction_ = &transaction;
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::detach()
{
  if( attached() ){
    free();
    if( transaction_ != NULL ){
      transaction_->dsqlStatements_.removeByObject(this);
      transaction_ = NULL;
    }
    if( database_ != NULL ){
      database_->dsqlStatements_.removeByObject(this);
      database_ = NULL;
    }
    prepared_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::allocate()
{
  if( !attached() )
    throw ksys::ExceptionSP(new EDSQLStNotAttached(NULL, __PRETTY_FUNCTION__));
  if( !allocated() ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_dsql_allocate_statement(status, &database_->handle_, &handle_) != 0 )
      database_->exceptionHandler(new EDSQLStAllocate(status, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::describe(XSQLDAHolder & sqlda)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_describe(status, &handle_, (short) database_->dpb_.dialect(), sqlda.sqlda()) != 0 )
    database_->exceptionHandler(new EDSQLStDescribe(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::describeBind(XSQLDAHolder & sqlda)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_describe_bind(status, &handle_, (short) database_->dpb_.dialect(), sqlda.sqlda()) != 0 )
    database_->exceptionHandler(new EDSQLStDescribeBind(status, __PRETTY_FUNCTION__));
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
        if( i2 - i > 1 ){
          params_.indexToParam_.add(params_.add(utf8::String(i + 1, i2)));
          text = text.replace(i, i2, "?");
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
    ISC_STATUS_ARRAY  status;
    for( ; ; ){
      if( api.isc_dsql_prepare(status, &transaction_->handle_, &handle_, 0, compileSQLParameters().c_str(), (short) database_->dpb_.dialect(), values_.sqlda_.sqlda()) == 0 )
        break;
      if( !findISCCode(status, isc_dsql_open_cursor_request) )
        database_->exceptionHandler(new EDSQLStPrepare(status, __PRETTY_FUNCTION__));
      dropCursor();
    }
    info();
    values_.clear().valuesIndex_.clear();
    if( stmtType == stmtSelect || stmtType == stmtSelectForUpd || stmtType == stmtExecProcedure ){
      describe(values_.sqlda_);
      if( values_.sqlda_.count() != (uintptr_t) values_.sqlda_.sqlda()->sqld ){
        bool  nd  = values_.sqlda_.count() < (uintptr_t) values_.sqlda_.sqlda()->sqld;
        values_.sqlda_.resize(values_.sqlda_.sqlda()->sqld);
        if( nd )
          describe(values_.sqlda_);
      }
    }
    // bind input parameters
    describeBind(params_.sqlda_);
    if( params_.sqlda_.count() != (uintptr_t) params_.sqlda_.sqlda()->sqld ){
      bool  nd  = params_.sqlda_.count() < (uintptr_t) params_.sqlda_.sqlda()->sqld;
      params_.sqlda_.resize(params_.sqlda_.sqlda()->sqld);
      if( nd )
        describeBind(params_.sqlda_);
    }
    params_.bind(DSQLParams::preBind);
    // bind input parameters
    sqlTextChanged_ = false;
    prepared_ = true;
  }
  else if( executed_ ){
    params_.bind(DSQLParams::preBindAfterExecute);
    executed_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::execute()
{
  transaction_->start();
  prepare();
  params_.bind(DSQLParams::postBind);
  for( ; ; ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_dsql_execute(status, &transaction_->handle_, &handle_, (short) database_->dpb_.dialect(), params_.sqlda_.sqlda()) == 0 )
      break;
    if( !findISCCode(status, isc_dsql_cursor_open_err) )
      database_->exceptionHandler(new EDSQLStExecute(status, __PRETTY_FUNCTION__));
    dropCursor();
  }
  values_.clear();
  if( transaction_->startCount_ == 1 && values_.sqlda_.count() > 0 )
    values_.fetchAll();
  transaction_->commitRetaining();
  executed_ = true;
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::createCursor(const utf8::String & name)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_set_cursor_name(status, &handle_, (char *) name.c_str(), 0) != 0 )
    database_->exceptionHandler(new EDSQLStCreateCursor(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::dropCursor()
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_free_statement(status, &handle_, DSQL_close) != 0 )
    database_->exceptionHandler(new EDSQLStDropCursor(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
ISC_STATUS DSQLStatement::fetch(ISC_STATUS_ARRAY status)
{
  ISC_STATUS  code;
  code = api.isc_dsql_fetch(status, &handle_, (short) database_->dpb_.dialect(), values_.sqlda_.sqlda());
  if( code != 100 && code != isc_req_sync && code != 0 )
    database_->exceptionHandler(new EDSQLStFetch(status, __PRETTY_FUNCTION__));
  return code;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::info()
{
  static char       stmtInfo[]  = {
    isc_info_sql_stmt_type
  };
  char              infoBuffer[20];
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_sql_info(status, &handle_, sizeof(stmtInfo), stmtInfo, sizeof(infoBuffer), infoBuffer) != 0 )
    database_->exceptionHandler(new EDSQLStInfo(status, __PRETTY_FUNCTION__));
  short l = (short) api.isc_vax_integer(infoBuffer + 1, 2);
  stmtType = (Stmt) api.isc_vax_integer(infoBuffer + 3, l);
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayLookupBounds(char * tableName, char * arrayColumnName, ISC_ARRAY_DESC & desc)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_lookup_bounds(status, &database_->handle_, &transaction_->handle_, tableName, arrayColumnName, &desc) != 0 )
    database_->exceptionHandler(new EDSQLStArrayLookupBounds(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayPutSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_put_slice(status, &database_->handle_, &transaction_->handle_, &arrayId, &desc, data, &count) != 0 )
    database_->exceptionHandler(new EDSQLStArrayPutSlice(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayGetSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_get_slice(status, &database_->handle_, &transaction_->handle_, &arrayId, &desc, data, &count) != 0 )
    database_->exceptionHandler(new EDSQLStArrayGetSlice(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::createBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, short bpbLength, char * bpbAddress)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_create_blob2(status, &database_->handle_, &transaction_->handle_, &blobHandle, &blobId, bpbLength, bpbAddress) != 0 )
    database_->exceptionHandler(new EDSQLStCreateBlob(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::openBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, ISC_USHORT bpbLength, ISC_UCHAR * bpbAddress)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_open_blob2(status, &database_->handle_, &transaction_->handle_, &blobHandle, &blobId, bpbLength, bpbAddress) != 0 )
    database_->exceptionHandler(new EDSQLStOpenBlob(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::closeBlob(isc_blob_handle & blobHandle)
{
  if( blobHandle != 0 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_close_blob(status, &blobHandle) != 0 )
      database_->exceptionHandler(new EDSQLStCloseBlob(status, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::cancelBlob(isc_blob_handle & blobHandle)
{
  if( blobHandle != 0 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_cancel_blob(status, &blobHandle) != 0 )
      database_->exceptionHandler(new EDSQLStCancelBlob(status, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::blobLookupDesc(char * tableName, char * columnName, ISC_BLOB_DESC & desc, char * globalColumnName)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_blob_lookup_desc(status, &database_->handle_, &transaction_->handle_, tableName, columnName, &desc, globalColumnName) != 0 )
    database_->exceptionHandler(new EDSQLStPutSegment(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::free()
{
  if( allocated() ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_dsql_free_statement(status, &handle_, DSQL_drop) != 0 && status[1] != isc_bad_stmt_handle )
      database_->exceptionHandler(new EDSQLStFree(status, __PRETTY_FUNCTION__));
    handle_ = 0;
  }
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
  stmtType = stmtUnknown;
  return *this;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
