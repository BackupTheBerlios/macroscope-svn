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
Database::~Database()
{
  if( transaction_ != NULL ) transaction_->detach();
  detach();
}
//---------------------------------------------------------------------------
Database::Database() : envHandle_(NULL), handle_(NULL),  transaction_(NULL)
{
}
//---------------------------------------------------------------------------
Database & Database::freeHandle()
{
  if( handle_ != NULL ){
    api.SQLFreeHandle(SQL_HANDLE_DBC,handle_);
    handle_ = NULL;
  }
  if( envHandle_ != NULL ){
    api.SQLFreeHandle(SQL_HANDLE_ENV,envHandle_);
    envHandle_ = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
EClientServer * Database::exception(SQLSMALLINT handleType,SQLHANDLE handle,utf8::String * pSqlState) const
{
  SQLWCHAR sqlState[6], msg[SQL_MAX_MESSAGE_LENGTH];
  SQLINTEGER nativeError;
  SQLSMALLINT msgLen, recNumber;
  SQLRETURN r = SQL_SUCCESS;
  ksys::AutoPtr<EClientServer> e(newObject<EClientServer>());
  for( recNumber = 1; r != SQL_NO_DATA; recNumber++ ){
    sqlState[0] = L'\0';
    r = api.SQLGetDiagRecW(
      handleType,
      handle,
      recNumber,
      sqlState,
      &nativeError,
      msg,
      sizeof(msg),
      &msgLen
    );
    if( r != SQL_NO_DATA ){
      if( pSqlState != NULL ){ *pSqlState = sqlState; pSqlState = NULL; }
      e->addError(nativeError == 0 ? EINVAL : nativeError,"ODBC state: " + utf8::String(sqlState) +
        (nativeError != 0 ? ", native error code: " + utf8::int2Str(nativeError) : utf8::String()) +
        (msgLen > 0 ? utf8::String(", ") + msg : utf8::String())
      );
    }
  }
  return e.ptr(NULL);
}
//---------------------------------------------------------------------------
Database & Database::create(const utf8::String &)
{
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::drop()
{
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::attach(const utf8::String & name)
{
  if( !attached() ){
    api.open();
    try {
      SQLRETURN r = api.SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&envHandle_);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        exceptionHandler(newObjectV1C2<EClientServer>(EINVAL,"ODBC SQLAllocHandle(SQL_HANDLE_ENV) failed " + utf8::String(__PRETTY_FUNCTION__)));
      r = api.SQLSetEnvAttr(envHandle_,SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3,0);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        exceptionHandler(exception(SQL_HANDLE_ENV,envHandle_));
      r = api.SQLAllocHandle(SQL_HANDLE_DBC,envHandle_,&handle_);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        exceptionHandler(exception(SQL_HANDLE_ENV,envHandle_));
      /*r = api.SQLSetConnectAttr(handle_,SQL_LOGIN_TIMEOUT,(SQLPOINTER) 5,0);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        exceptionHandler(exception(SQL_HANDLE_DBC,handle_));*/
      ksys::AutoPtr<SQLWCHAR> connOut;
      connOut.alloc(sizeof(SQLWCHAR) * SQLSMALLINT((1u << (sizeof(SQLSMALLINT) * 8 - 1)) - 1));
      SQLSMALLINT cbConnStrOut = 0;
      r = api.SQLBrowseConnectW(
        handle_,
        (name.isNull() ? connection_ : name).getUNICODEString(),
        SQL_NTS,
        connOut,
        SQLSMALLINT((1u << (sizeof(SQLSMALLINT) * 8 - 1)) - 1),
        &cbConnStrOut
      );
      if( r == SQL_NEED_DATA ){
        exceptionHandler(newObjectV1C2<EClientServer>(EINVAL,
          utf8::String("ODBC SQLBrowseConnect failed, ") + connOut.ptr() + ", " + __PRETTY_FUNCTION__));
      }
      else if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
        exceptionHandler(exception(SQL_HANDLE_DBC,handle_));
    }
    catch( ksys::ExceptionSP & ){
      freeHandle();
      api.close();
      throw;
    }
    if( !name.isNull() ) connection_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::detach()
{
  if( attached() ){
    if( transaction_ != NULL )
      while( transaction_->active() ) transaction_->rollback();
    ksys::EmbeddedListNode<DSQLStatement> * node = statements_.first();
    while( node != NULL ){
      DSQLStatement::listObject(*node).freeHandle();
      node = node->next();
    }
    freeHandle();
    api.close();
  }
  return *this;
}
//---------------------------------------------------------------------------
void Database::processingException(ksys::Exception * e)
{
  EClientServer * p = dynamic_cast<EClientServer *>(e);
  if( p != NULL ){
    if( p->isFatalError() ){
      detach();
    }
  }
}
//---------------------------------------------------------------------------
void Database::staticExceptionHandler(ksys::Exception * e)
{
  transaction_->processingException(e);
  processingException(e);
  e->throwSP();
}
//---------------------------------------------------------------------------
void Database::exceptionHandler(ksys::Exception * e)
{
  staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// properties ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Database & Database::connection(const utf8::String & connection)
{
  if( connection_.strcasecmp(connection) != 0 ){
    if( attached() ) detach();
    connection_ = connection;
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
