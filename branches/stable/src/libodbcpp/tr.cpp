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
Transaction::Transaction() : database_(NULL), startCount_(0)
{
}
//---------------------------------------------------------------------------
Transaction::~Transaction()
{
  detach();
}
//---------------------------------------------------------------------------
Transaction & Transaction::attach(Database & database)
{
  if( database_ != NULL && database_ != &database ) detach();
  if( database.transaction_ != NULL ) database.transaction_->detach();
  database_ = &database;
  database.transaction_ = this;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::detach()
{
  while( active() ) rollback();
  if( database_ != NULL ) database_->transaction_ = NULL;
  database_ = NULL;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::start()
{
#if HAVE_SQL_H
  if( !attached() )
    newObjectV1C2<EClientServer>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  if( startCount_ == 0 ){
    SQLRETURN r = SQL_SUCCESS;
    if( isolation_.strcasecmp("REPEATABLE") == 0 )
      r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER) SQL_TXN_REPEATABLE_READ,0);
    else if( isolation_.strcasecmp("READ_UNCOMMITTED") == 0 )
      r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER) SQL_TXN_READ_UNCOMMITTED,0);
    else if( isolation_.strcasecmp("READ_COMMITTED") == 0 )
      r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER) SQL_TXN_READ_COMMITTED,0);
    else if( isolation_.strcasecmp("SERIALIZABLE") == 0 )
      r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER) SQL_TXN_SERIALIZABLE,0);
    else
      r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_TXN_ISOLATION,(SQLPOINTER) SQL_TXN_READ_COMMITTED,0);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_DBC,database_->handle_));
    r = api.SQLSetConnectAttr(database_->handle_,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER) SQL_AUTOCOMMIT_OFF,0);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_DBC,database_->handle_));
  }
  startCount_++;
#else
  newObjectV1C2<EClientServer>(ENOSYS, __PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::commit()
{
#if HAVE_SQL_H
  if( !active() )
    newObjectV1C2<EClientServer>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  assert( startCount_ > 0 );
  if( startCount_ == 1 ){
    SQLRETURN r = api.SQLEndTran(SQL_HANDLE_DBC,database_->handle_,SQL_COMMIT);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_DBC,database_->handle_));
  }
  startCount_--;
#else
  newObjectV1C2<EClientServer>(ENOSYS, __PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::rollback()
{
#if HAVE_SQL_H
  if( !active() )
    newObjectV1C2<EClientServer>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  assert( startCount_ > 0 );
  if( startCount_ == 1 ){
    SQLRETURN r = api.SQLEndTran(SQL_HANDLE_DBC,database_->handle_,SQL_ROLLBACK);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
      database_->exceptionHandler(database_->exception(SQL_HANDLE_DBC,database_->handle_));
  }
  startCount_--;
#else
  newObjectV1C2<EClientServer>(ENOSYS, __PRETTY_FUNCTION__)->throwSP();
#endif
  return *this;
}
//---------------------------------------------------------------------------
void Transaction::processingException(ksys::Exception * e)
{
  EClientServer * p = dynamic_cast< EClientServer *>(e);
  if( p != NULL ){
    if( p->isFatalError() ){
      startCount_ = 0;
    }
    else {
      if( active() ) rollback();
    }
  }
}
//---------------------------------------------------------------------------
void Transaction::staticExceptionHandler(ksys::Exception * e)
{
  processingException(e);
  database_->processingException(e);
  e->throwSP();
}
//---------------------------------------------------------------------------
void Transaction::exceptionHandler(ksys::Exception * e)
{
  staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
