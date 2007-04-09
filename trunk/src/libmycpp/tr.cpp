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
#include <adicpp/mycpp.h>
//---------------------------------------------------------------------------
namespace mycpp {
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
  if( !attached() )
    newObjectV1C2<ETrNotAttached>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  if( startCount_ == 0 ){
    utf8::String trSQL("SET SESSION TRANSACTION ISOLATION LEVEL ");
    if( isolation_.strcasecmp("REPEATABLE") == 0 )
      trSQL += "REPEATABLE READ";
    else if( isolation_.strcasecmp("READ_COMMITTED") == 0 )
      trSQL += "READ COMMITTED";
    else if( isolation_.strcasecmp("SERIALIZABLE") == 0 )
      trSQL += "SERIALIZABLE";
//    else if( isolation_.strcasecmp("SNAPSHOT") == 0 )
//      trSQL += " WITH CONSISTENT SNAPSHOT";
    else
      trSQL += "READ COMMITTED";
    if( api.mysql_query(database_->handle_,trSQL.c_str()) != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStExecute>(
        api.mysql_errno(database_->handle_),api.mysql_error(database_->handle_)));
    if( api.mysql_query(database_->handle_,"START TRANSACTION") != 0 )
      database_->exceptionHandler(newObjectV1C2<EDSQLStExecute>(
        api.mysql_errno(database_->handle_),api.mysql_error(database_->handle_)));
  }
  startCount_++;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::commit()
{
  if( !active() )
    newObjectV1C2<ETrNotActive>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  assert(startCount_ > 0);
  if( startCount_ == 1 ){
    if( api.mysql_commit(database_->handle_) != 0 && api.mysql_errno(database_->handle_) != CR_SERVER_GONE_ERROR )
      exceptionHandler(newObjectV1C2<ETrCommit>(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
  }
  startCount_--;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::rollback()
{
  if( !active() )
    newObjectV1C2<ETrNotActive>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  assert(startCount_ > 0);
  if( startCount_ == 1 ){
    if( api.mysql_rollback(database_->handle_) != 0 && api.mysql_errno(database_->handle_) != CR_SERVER_GONE_ERROR )
      exceptionHandler(newObjectV1C2<ETrRollback>(
        api.mysql_errno(database_->handle_), api.mysql_error(database_->handle_)));
  }
  startCount_--;
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
    else{
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
} // namespace mycpp
//---------------------------------------------------------------------------
