/*-
 * Copyright 2005-2008 Guram Dukashvili
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
const int64_t iscTimeOffset = INT64_C(3506727600) - INT64_C(10800);
//---------------------------------------------------------------------------
int64_t iscTimeStamp2Time(const ISC_TIMESTAMP & stamp)
{
  int64_t t = stamp.timestamp_date;
  t = 
    (t << 16) +
    (t << 14) +
    (t << 12) +
    (t << 8) +
    (t << 7) +
    stamp.timestamp_time / ISC_TIME_SECONDS_PRECISION - iscTimeOffset;
  t *= 1000000;
  t += (stamp.timestamp_time % ISC_TIME_SECONDS_PRECISION) * 100;
  return t;
}
//---------------------------------------------------------------------------
struct timeval iscTimeStamp2Timeval(const ISC_TIMESTAMP & stamp)
{
  struct timeval tv;
  int64_t t = stamp.timestamp_date;
  tv.tv_sec = long(
    (t << 16) + 
    (t << 14) + 
    (t << 12) + 
    (t << 8) + 
    (t << 7) +
    stamp.timestamp_time / ISC_TIME_SECONDS_PRECISION - iscTimeOffset);
  tv.tv_usec = (stamp.timestamp_time % ISC_TIME_SECONDS_PRECISION) * 100;
  return tv;
}
//---------------------------------------------------------------------------
struct tm iscTimeStamp2tm(const ISC_TIMESTAMP & stamp)
{
  time_t t =
    (stamp.timestamp_date << 16) +
    (stamp.timestamp_date << 14) +
    (stamp.timestamp_date << 12) +
    (stamp.timestamp_date << 8) +
    (stamp.timestamp_date << 7) +
    stamp.timestamp_time / ISC_TIME_SECONDS_PRECISION - iscTimeOffset;
  return ksys::time2tm(t * 1000000u);
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP time2IscTimeStamp(int64_t stamp)
{
  ISC_TIMESTAMP iscStamp;
  int64_t t = stamp / 1000000 + iscTimeOffset;
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * ISC_TIME_SECONDS_PRECISION + (stamp % 1000000) / 100);
  return iscStamp;
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP timeval2IscTimeStamp(const struct timeval & stamp)
{
  ISC_TIMESTAMP iscStamp;
  int64_t t = stamp.tv_sec + iscTimeOffset;
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * ISC_TIME_SECONDS_PRECISION + stamp.tv_usec / 100);
  return iscStamp;
}
//---------------------------------------------------------------------------
ISC_TIMESTAMP tm2IscTimeStamp(struct tm stamp)
{
  ISC_TIMESTAMP iscStamp;
  int64_t t = timegm(&stamp) + iscTimeOffset;
  iscStamp.timestamp_date = ISC_DATE(t / 86400);
  iscStamp.timestamp_time = ISC_TIME((t % 86400) * ISC_TIME_SECONDS_PRECISION);
  return iscStamp;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
XSQLDAHolder::XSQLDAHolder() : sqlda_(NULL)
{
  sqlda_ = (XSQLDA *) ksys::kmalloc(sizeof (XSQLDA) - sizeof (XSQLVAR));//XSQLDA_LENGTH(0))
  sqlda_->sqln = 0;
  sqlda_->version = SQLDA_VERSION1;
}
//---------------------------------------------------------------------------
XSQLDAHolder::~XSQLDAHolder()
{
  ksys::kfree(sqlda_);
}
//---------------------------------------------------------------------------
XSQLDAHolder & XSQLDAHolder::resize(long n)
{
  if( n < 0 ) n = 0;
  sqlda_ = (XSQLDA *) ksys::krealloc(sqlda_,XSQLDA_LENGTH(n));
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
    plan_(true),
    stmtType(stmtUnknown)
{
  params_.statement_ = this;
  values_.statement_ = this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::attach(Database & database, Transaction & transaction)
{
  if( attached() )
    database.exceptionHandler(newObjectV1C2<EDSQLStAttached>((ISC_STATUS *) NULL,__PRETTY_FUNCTION__));
  if( database.transactions_.itemOfObject(&transaction) == NULL )
    database.exceptionHandler(newObjectV1C2<EDSQLStNotAttached>((ISC_STATUS *) NULL,__PRETTY_FUNCTION__));
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
    newObjectV1C2<EDSQLStNotAttached>((ISC_STATUS *) NULL,__PRETTY_FUNCTION__)->throwSP();
  if( !allocated() ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_dsql_alloc_statement2(status, &database_->handle_, &handle_) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStAllocate>(status,__PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::describe(XSQLDAHolder & sqlda)
{
  ISC_STATUS_ARRAY status;
  if( api.isc_dsql_describe(status, &handle_, (short) database_->dpb_.dialect(), sqlda.sqlda()) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStDescribe>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::describeBind(XSQLDAHolder & sqlda)
{
  ISC_STATUS_ARRAY status;
  if( api.isc_dsql_describe_bind(status, &handle_, (short) database_->dpb_.dialect(), sqlda.sqlda()) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStDescribeBind>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
utf8::String DSQLStatement::getPreparedSqlText()
{
  utf8::String text(sqlText_.unique());
  utf8::String::Iterator i(text);
  while( !i.eos() ){
    uintptr_t c = i.getChar();
    if( c == ':' ){
      utf8::String::Iterator i2(i);
      while( i2.next() && ((c = i2.getChar()) == '_' || (utf8::getC1Type(c) & (C1_ALPHA | C1_DIGIT)) != 0) );
      if( i2 - i > 1 && !(i + 1).isDigit() ) text.replace(i,i2,"?");
    }
    i.next();
  }
  return text;
}
//---------------------------------------------------------------------------
utf8::String DSQLStatement::compileSQLParameters()
{
  params_.params_.drop();
  params_.indexToParam_.clear();
  if( sqlText_.ncasecompare("CREATE PROCEDURE ",17) == 0 || sqlText_.ncasecompare("ALTER PROCEDURE ",16) == 0 )
    return sqlText_;
  utf8::String text(sqlText_.unique());
  utf8::String::Iterator i(text);
  while( !i.eos() ){
    uintptr_t c = i.getChar();
    if( c == ':' ){
      utf8::String::Iterator i2(i);
      while( i2.next() && ((c = i2.getChar()) == '_' || (utf8::getC1Type(c) & (C1_ALPHA | C1_DIGIT)) != 0) );
      if( i2 - i > 1 ){
        DSQLParam * param;
        ksys::AutoPtr<DSQLParam> p(newObjectV1C2<DSQLParam>(this,utf8::String(i + 1,i2)));
        params_.params_.insert(p,false,false,&param);
        p->head_ = param;
        if( p.ptr() != param ){
          p->name_ = param->name_;
          p->next_ = param->next_;
          param->next_ = p;
        }
	      p->index_ = params_.indexToParam_.count();
        params_.indexToParam_.add(p.ptr(NULL));
        text.replace(i,i2,"?");
      }
    }
    i.next();
  }
  return text;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::prepare()
{
  allocate();
  if( sqlTextChanged_ || !prepared_ ){
    dropCursor();
    ISC_STATUS_ARRAY status;
    if( api.isc_dsql_prepare(status, &transaction_->handle_,&handle_,0,compileSQLParameters().c_str(),(short) database_->dpb_.dialect(),values_.sqlda_.sqlda()) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStPrepare>(status,sqlText_ + " " + __PRETTY_FUNCTION__));
    info();
    values_.clear().valuesIndex_.clear();
    if( stmtType == stmtSelect || stmtType == stmtSelectForUpd || stmtType == stmtExecProcedure ){
      describe(values_.sqlda_);
      if( values_.sqlda_.count() != (uintptr_t) values_.sqlda_.sqlda()->sqld ){
        bool nd  = values_.sqlda_.count() < (uintptr_t) values_.sqlda_.sqlda()->sqld;
        values_.sqlda_.resize(values_.sqlda_.sqlda()->sqld);
        if( nd ) describe(values_.sqlda_);
      }
    }
    else {
      values_.sqlda_.resize(0);
    }
    // bind input parameters
    describeBind(params_.sqlda_);
    if( params_.sqlda_.count() != (uintptr_t) params_.sqlda_.sqlda()->sqld ){
      bool nd = params_.sqlda_.count() < (uintptr_t) params_.sqlda_.sqlda()->sqld;
      params_.sqlda_.resize(params_.sqlda_.sqlda()->sqld);
      if( nd ) describeBind(params_.sqlda_);
    }
    // bind input parameters
    sqlTextChanged_ = false;
    prepared_ = true;
  }
  else if( executed_ ){
    executed_ = false;
  }
  params_.bind();
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::execute()
{
  transaction_->start();
  try {
    prepare();
    dropCursor();
    ISC_STATUS_ARRAY status;
    if( api.isc_dsql_execute(status, &transaction_->handle_, &handle_, (short) database_->dpb_.dialect(), params_.sqlda_.sqlda()) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStExecute>(status,sqlText_ + " " + __PRETTY_FUNCTION__));
    values_.clear();
    if( transaction_->startCount_ == 1 && values_.sqlda_.count() > 0 )
      values_.fetchAll();
    if( plan_ && ksys::stdErr.debugLevel(160) )
      ksys::stdErr.debug(160,utf8::String::Stream() <<
        "Plan for statement:\n" <<
        sqlText_ << "\n" <<
        "Plan:\n" <<
        plan() << "\n"
      );
  }
  catch( ksys::ExceptionSP & e ){
    transaction_->rollbackRetaining();
    throw e;
  }
  transaction_->commitRetaining();
  executed_ = true;
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::createCursor(const utf8::String & name)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_set_cursor_name(status, &handle_, (char *) name.c_str(), 0) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStCreateCursor>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::dropCursor()
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_free_statement(status, &handle_, DSQL_close) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStDropCursor>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
ISC_STATUS DSQLStatement::fetch(ISC_STATUS_ARRAY status)
{
  ISC_STATUS code;
  code = api.isc_dsql_fetch(status, &handle_, (short) database_->dpb_.dialect(), values_.sqlda_.sqlda());
  if( code != 100 && code != isc_req_sync && code != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStFetch>(status,sqlText_ + " " + __PRETTY_FUNCTION__));
  return code;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::info()
{
  static char stmtInfo[] = { isc_info_sql_stmt_type };
  char infoBuffer[20];
  ISC_STATUS_ARRAY  status;
  if( api.isc_dsql_sql_info(status, &handle_, sizeof(stmtInfo), stmtInfo, sizeof(infoBuffer), infoBuffer) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStInfo>(status, __PRETTY_FUNCTION__));
  short l = (short) api.isc_vax_integer(infoBuffer + 1, 2);
  stmtType = (Stmt) api.isc_vax_integer(infoBuffer + 3, l);
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayLookupBounds(char * tableName, char * arrayColumnName, ISC_ARRAY_DESC & desc)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_lookup_bounds(status, &database_->handle_, &transaction_->handle_, tableName, arrayColumnName, &desc) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStArrayLookupBounds>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayPutSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_put_slice(status, &database_->handle_, &transaction_->handle_, &arrayId, &desc, data, &count) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStArrayPutSlice>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::arrayGetSlice(ISC_QUAD & arrayId, ISC_ARRAY_DESC & desc, void * data, ISC_LONG & count)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_array_get_slice(status, &database_->handle_, &transaction_->handle_, &arrayId, &desc, data, &count) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStArrayGetSlice>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::createBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, short bpbLength, char * bpbAddress)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_create_blob2(status, &database_->handle_, &transaction_->handle_, &blobHandle, &blobId, bpbLength, bpbAddress) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStCreateBlob>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::openBlob(isc_blob_handle & blobHandle, ISC_QUAD & blobId, ISC_USHORT bpbLength, ISC_UCHAR * bpbAddress)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_open_blob2(status, &database_->handle_, &transaction_->handle_, &blobHandle, &blobId, bpbLength, bpbAddress) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStOpenBlob>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::closeBlob(isc_blob_handle & blobHandle)
{
  if( blobHandle != 0 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_close_blob(status, &blobHandle) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStCloseBlob>(status, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::cancelBlob(isc_blob_handle & blobHandle)
{
  if( blobHandle != 0 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_cancel_blob(status, &blobHandle) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStCancelBlob>(status, __PRETTY_FUNCTION__));
  }
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::blobLookupDesc(char * tableName, char * columnName, ISC_BLOB_DESC & desc, char * globalColumnName)
{
  ISC_STATUS_ARRAY  status;
  if( api.isc_blob_lookup_desc(status, &database_->handle_, &transaction_->handle_, tableName, columnName, &desc, globalColumnName) != 0 )
    database_->exceptionHandler(newObjectV1C2<EDSQLStPutSegment>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
DSQLStatement & DSQLStatement::free()
{
  if( allocated() ){
    ISC_STATUS_ARRAY status;
    if( api.isc_dsql_free_statement(status,&handle_,DSQL_drop) != 0 && status[1] != isc_bad_stmt_handle ){
      ksys::AutoPtr<EDSQLStFree> e(newObjectV1C2<EDSQLStFree>(status,__PRETTY_FUNCTION__));
      if( !e->isFatalError() ) database_->exceptionHandler(e.ptr(NULL));
    }
    handle_ = 0;
    prepared_ = false;
  }
  return *this;
}
//---------------------------------------------------------------------------
char * findToken(char * mBuffer,char token)
{
  char * p = mBuffer;
  while( *p != isc_info_end ){
    int len;
    if( *p == token ) return p;
    len = api.isc_vax_integer(p + 1,2);
    p += (len + 3);
  }
  return 0;
}
//---------------------------------------------------------------------------
char * findToken(char * mBuffer,char token,char subToken)
{
  if( subToken == -1 ) return findToken(mBuffer,token);

  char * p = mBuffer;
	while( *p != isc_info_end ){
		int len;
		if( *p == token ){
			// Found token, now find subtoken
			int inlen = api.isc_vax_integer(p + 1,2);
			p = p + 3;
			while( inlen > 0 ){
				if( *p == subToken ) return p;
				len = api.isc_vax_integer(p + 1,2);
				p = p + len + 3;
				inlen = inlen - len - 3;
			}
			return NULL;
		}
		len = api.isc_vax_integer(p + 1,2);
		p = p + len + 3;
	}
	return NULL;
}
//---------------------------------------------------------------------------
utf8::String getTokenString(char * mBuffer,char token,char subToken)
{
  int len;
  char * p = findToken(mBuffer,token,subToken);
  if( p != NULL ){
    len = api.isc_vax_integer(p + 1,2);
    if( p[3] == '\n' ){
      p += 4;
      len--;
    }
    else {
      p += 3;
    }
    return utf8::plane(p,len);
  }
  return utf8::String();
}
//---------------------------------------------------------------------------
int getTokenValue(char * mBuffer,char token,char subToken)
{
  int value = 0, len;
  char * p = findToken(mBuffer,token,subToken);
  if( p != NULL ){
    len = api.isc_vax_integer(p + 1,2);
    while( len > 0 ){
  		value += api.isc_vax_integer(p + 2,4);
	  	p += 6;
		  len -= 6;
    }
  }
  return value;
}
//---------------------------------------------------------------------------
utf8::String DSQLStatement::plan()
{
  utf8::String info;
  if( plan_ ){
    utf8::String infoBuffer;
    infoBuffer.resize(32767);
    static char stmtInfo[] = { isc_info_sql_get_plan/*, isc_info_sql_records*/ };
    ISC_STATUS_ARRAY status;
    if( api.isc_dsql_sql_info(status, &handle_, sizeof(stmtInfo), stmtInfo, 32767, infoBuffer.c_str()) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStInfo>(status, __PRETTY_FUNCTION__));
    info = getTokenString(infoBuffer.c_str(),isc_info_sql_get_plan);
    //static char stmtInfo2[] = { isc_info_sql_records };
    //if( !info.isNull() ) info += "\n";
    //info += "Statement info:\n";
    //info += "inserted: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_sql_records,isc_info_req_insert_count));
    //info += ", updated: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_sql_records,isc_info_req_update_count));
    //info += ", deleted: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_sql_records,isc_info_req_delete_count));
    //info += ", selected: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_sql_records,isc_info_req_select_count));
    
    DSQLStatement st;
    st.plan_ = false;
    st.attach(*database_,*transaction_);
    st.sqlText(
      "SELECT\n"
      " io.MON$PAGE_READS,\n"
      " io.MON$PAGE_WRITES,\n"
      " io.MON$PAGE_FETCHES,\n"
      " io.MON$PAGE_MARKS,\n"
      " st.MON$SQL_TEXT\n"
      "FROM\n"
      " MON$TRANSACTIONS tr\n"
      " LEFT JOIN MON$STATEMENTS st\n"
      "   ON st.MON$TRANSACTION_ID = tr.MON$TRANSACTION_ID\n"
      " LEFT JOIN MON$IO_STATS io\n"
      "   ON io.MON$STAT_ID = st.MON$STAT_ID\n"
      "WHERE tr.MON$TRANSACTION_ID = CURRENT_TRANSACTION\n"
    ).execute().values_.fetchAll();
    utf8::String text(getPreparedSqlText());
    for( intptr_t i = st.values_.rowCount() - 1; i >= 0; i-- ){
      st.values_.selectRow(i);
      if( st.valueAsString("MON$SQL_TEXT").compare(text) == 0 ){
        if( !info.isNull() ) info += "\n";
        info += "page reads: " + st.valueAsString("MON$PAGE_READS");
        info += ", page writes: " + st.valueAsString("MON$PAGE_WRITES");
        info += ", page fetches: " + st.valueAsString("MON$PAGE_FETCHES");
        info += ", page marks: " + st.valueAsString("MON$PAGE_MARKS");
        break;
      }
    }
    //static char trId[] = { isc_info_tra_id };
    //if( api.isc_transaction_info(status, &transaction_->handle_, sizeof(trId), trId, 32767, infoBuffer.c_str()) != 0 )
    //  database_->exceptionHandler(newObjectV1C2<EDSQLStInfo>(status, __PRETTY_FUNCTION__));
    //int tid = getTokenValue(infoBuffer.c_str(),isc_info_tra_id);

    //static char items[] = {
    //  isc_info_read_seq_count,
    //  isc_info_read_idx_count,
    //  isc_info_insert_count,
    //  isc_info_update_count,
    //  isc_info_delete_count,
    //  isc_info_fetches,
		  //isc_info_marks,
		  //isc_info_reads,
		  //isc_info_writes,
		  //isc_info_end
    //};
    //if( api.isc_database_info(status, &database_->handle_, sizeof(items), items, 32767, infoBuffer.c_str()) != 0 )
    //  database_->exceptionHandler(newObjectV1C2<EDSQLStInfo>(status, __PRETTY_FUNCTION__));
    //if( !info.isNull() ) info += "\n";
    //info += "Database info:\n";
    //info += "read_seq: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_read_seq_count));
    //info += ", read_idx: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_read_idx_count));
    //info += ", inserted: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_insert_count));
    //info += ", updated: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_update_count));
    //info += ", deleted: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_delete_count));
    //info += ", fetches: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_fetches));
    //info += ", marks: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_marks));
    //info += ", reads: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_reads));
    //info += ", writes: " + utf8::int2Str(getTokenValue(infoBuffer.c_str(),isc_info_writes));
  }
  return info;
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
