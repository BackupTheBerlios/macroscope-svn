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
TPB::TPBParam TPB::params[] = {
  {             "version1",               isc_tpb_version1 },
  {             "version3",               isc_tpb_version3 },
  {          "consistency",            isc_tpb_consistency },
  {          "concurrency",            isc_tpb_concurrency },
  {               "shared",                 isc_tpb_shared },
  {            "protected",              isc_tpb_protected },
  {            "exclusive",              isc_tpb_exclusive },
  {                 "wait",                   isc_tpb_wait },
  {               "nowait",                 isc_tpb_nowait },
  {                 "read",                   isc_tpb_read },
  {                "write",                  isc_tpb_write },
  {            "lock_read",              isc_tpb_lock_read },
  {           "lock_write",             isc_tpb_lock_write },
  {            "verb_time",              isc_tpb_verb_time },
  {          "commit_time",            isc_tpb_commit_time },
  {         "ignore_limbo",           isc_tpb_ignore_limbo },
  {       "read_committed",         isc_tpb_read_committed },
  {           "autocommit",             isc_tpb_autocommit },
  {          "rec_version",            isc_tpb_rec_version },
  {       "no_rec_version",         isc_tpb_no_rec_version },
  {     "restart_requests",       isc_tpb_restart_requests },
  {         "no_auto_undo",           isc_tpb_no_auto_undo }
};
//---------------------------------------------------------------------------
TPB::TPB() : tpb_(NULL), tpbLen_(0)
{
}
//---------------------------------------------------------------------------
TPB::~TPB()
{
  ksys::xfree(tpb_);
}
//---------------------------------------------------------------------------
TPB & TPB::clear()
{
  ksys::xfree(tpb_);
  tpb_ = NULL;
  tpbLen_ = 0;
  return *this;
}
//---------------------------------------------------------------------------
TPB & TPB::writeChar(uintptr_t code)
{
  ksys::xrealloc(tpb_, tpbLen_ + 1);
  tpb_[tpbLen_++] = char(code);
  return *this;
}
//---------------------------------------------------------------------------
intptr_t TPB::writeISCCode(const utf8::String & name)
{
  intptr_t  i;
  for( i = sizeof(params) / sizeof(params[0]) - 1; i >= 0; i-- ){
    if( name.strcasecmp(params[i].name_) == 0 ){
      ksys::xrealloc(tpb_, tpbLen_ + 1);
      tpb_[tpbLen_++] = params[i].number;
      return params[i].number;
    }
  }
  return i;
}
//---------------------------------------------------------------------------
TPB & TPB::add(const utf8::String & name)
{
  writeISCCode(name);
  return *this;
}
//---------------------------------------------------------------------------
TPB & TPB::add(const utf8::String & name, const utf8::String & tableName)
{
  writeISCCode(name);
  const char *  s = tableName.c_str();
  do
    writeChar(*s);
  while( *s != '\0' );
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Transaction::Transaction() : handle_(0), startCount_(0), lastRetainingTransaction_(lrtNone)
{
#ifndef __GNUG__
  databases_.ownsObjects(false);
#endif
  dsqlStatements_.ownsObjects(false);
  tpbs_.ownsObjects(true);
}
//---------------------------------------------------------------------------
Transaction::~Transaction()
{
  detach();
  while( dsqlStatements_.count() > 0 ){
    DSQLStatement * dsqlStatement = dsqlStatements_.objectOfIndex(0);
    dsqlStatement->free();
    dsqlStatement->transaction_ = NULL;
    dsqlStatements_.removeByIndex(0);
  }
}
//---------------------------------------------------------------------------
Transaction & Transaction::attach(Database & database)
{
  utf8::String dbKey(utf8::ptr2Str(&database));
  tpbs_.add(newObject<TPB>(),dbKey);
  try{
#if __GNUG__
    databases_.add(&database);
#else
    databases_.add(&database, dbKey);
#endif
    try{
      database.transactions_.add(this, utf8::ptr2Str(this));
    }
    catch( ksys::ExceptionSP & ){
#if __GNUG__
      databases_.resize(databases_.count() - 1);
#else
      databases_.removeByKey(dbKey);
#endif
      throw;
    }
  }
  catch( ksys::ExceptionSP & ){
    tpbs_.removeByKey(dbKey);
    throw;
  }
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::detach(Database & database)
{
  while( active() ) rollback();
  tpbs_.removeByKey(utf8::ptr2Str(&database));
  database.transactions_.removeByObject(this);
#if __GNUG__
  databases_.remove(databases_.search(&database));
#else  
  databases_.removeByObject(&database);
#endif
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::detach()
{
#if __GNUG__
  while( databases_.count() > 0 ) detach(*databases_[0]);
#else  
  while( databases_.count() > 0 ) detach(*databases_.objectOfIndex(0));
#endif
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::retainingHelper()
{
  ISC_STATUS_ARRAY  status;
  switch( lastRetainingTransaction_ ){
    case lrtNone :
      break;
    case lrtCommit :
      if( api.isc_commit_transaction(status, &handle_) != 0 )
        exceptionHandler(newObjectV1C2<ETrCommit>(status, __PRETTY_FUNCTION__));
      break;
    case lrtRollback :
      if( api.isc_rollback_transaction(status, &handle_) != 0 )
        exceptionHandler(newObjectV1C2<ETrRollback>(status, __PRETTY_FUNCTION__));
      break;
  }
  lastRetainingTransaction_ = lrtNone;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::isolation(const utf8::String & isolation)
{
  isolation_ = isolation;
  for( intptr_t i = databases_.count() - 1; i >= 0; i-- ){
#if __GNUG__
    TPB * tpb = tpbs_.objectOfKey(utf8::ptr2Str(databases_[i]));
#else
    TPB * tpb = tpbs_.objectOfKey(databases_.keyOfIndex(i));
#endif
    tpb->clear();
  }
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::start()
{
  if( !attached() )
    newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__);
  if( startCount_ == 0 ){
    retainingHelper(); // pumping retaining transactions
    ksys::AutoPtr<ISC_TEB> tebVector;
    tebVector.alloc(sizeof(ISC_TEB) * databases_.count());
    for( intptr_t i = databases_.count() - 1; i >= 0; i-- ){
#if __GNUG__
      tebVector[i].db_ptr = &databases_[i]->handle_;
      TPB * tpb = tpbs_.objectOfKey(utf8::ptr2Str(databases_[i]));
#else
      tebVector[i].db_ptr = &databases_.objectOfIndex(i)->handle_;
      TPB * tpb = tpbs_.objectOfKey(databases_.keyOfIndex(i));
#endif
      if( tpb->tpbLen_ == 0 ){
        if( isolation_.strcasecmp("REPEATABLE") == 0 ){
          tpb->add("version3");
          tpb->add("concurrency");
          //tpb->add("read");
          tpb->add("write");
          tpb->add("nowait");
        }
        else if( isolation_.strcasecmp("SERIALIZABLE") == 0 ){
          tpb->add("version3");
          tpb->add("consistency");
          //tpb->add("read");
          tpb->add("write");
          tpb->add("wait");
        }
        else if( isolation_.strcasecmp("READ_COMMITTED") == 0 ){
l1:       tpb->add("version3");
          tpb->add("read_committed");
          tpb->add("rec_version");
          tpb->add("nowait");
        }
        else if( isolation_.strcasecmp("READ_ONLY_COMMITTED") == 0 ){
          tpb->add("version3");
          tpb->add("read");
          tpb->add("read_committed");
          tpb->add("rec_version");
          tpb->add("nowait");
        }
        else
          goto l1;
      }
      tebVector[i].tpb_len = tpb->tpbLen_;
      tebVector[i].tpb_ptr = tpb->tpb_;
    }
    ISC_STATUS_ARRAY  status;
    if( api.isc_start_multiple(status, &handle_, (short) databases_.count(), tebVector.ptr()) != 0 )
      exceptionHandler(newObjectV1C2<ETrStart>(status, __PRETTY_FUNCTION__));
  }
  startCount_++;
  lastRetainingTransaction_ = lrtNone;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::prepare()
{
  if( !active() )
    exceptionHandler(newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__));
  ISC_STATUS_ARRAY  status;
  if( api.isc_prepare_transaction(status, &handle_) != 0 )
    exceptionHandler(newObjectV1C2<ETrPrepare>(status, __PRETTY_FUNCTION__));
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::commit()
{
  if( !active() )
    exceptionHandler(newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__));
  ISC_STATUS_ARRAY  status;
  if( startCount_ == 1 ){
    if( api.isc_commit_transaction(status, &handle_) != 0 )
      exceptionHandler(newObjectV1C2<ETrCommit>(status, __PRETTY_FUNCTION__));
    startCount_--;
  }
  else if( startCount_ == 0 ){
    retainingHelper();
  }
  else{
    startCount_--;
  }
  lastRetainingTransaction_ = lrtNone;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::commitRetaining()
{
  if( !active() )
    exceptionHandler(newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__));
  if( startCount_ == 1 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_commit_retaining(status, &handle_) != 0 )
      exceptionHandler(newObjectV1C2<ETrCommit>(status, __PRETTY_FUNCTION__));
    lastRetainingTransaction_ = lrtCommit;
    startCount_--;
  }
  else if( startCount_ == 0 ){
    retainingHelper();
  }
  else{
    startCount_--;
    lastRetainingTransaction_ = lrtNone;
  }
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::rollback()
{
  if( !active() )
    exceptionHandler(newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__));
  ISC_STATUS_ARRAY  status;
  if( startCount_ == 1 ){
    if( api.isc_rollback_transaction(status, &handle_) != 0 )
      exceptionHandler(newObjectV1C2<ETrRollback>(status, __PRETTY_FUNCTION__));
    startCount_--;
  }
  else if( startCount_ == 0 ){
    retainingHelper();
  }
  else{
    startCount_--;
  }
  lastRetainingTransaction_ = lrtNone;
  return *this;
}
//---------------------------------------------------------------------------
Transaction & Transaction::rollbackRetaining()
{
  if( !active() )
    exceptionHandler(newObjectV1C2<ETrNotActive>((ISC_STATUS *) NULL, __PRETTY_FUNCTION__));
  if( startCount_ == 1 ){
    ISC_STATUS_ARRAY  status;
    if( api.isc_rollback_retaining(status, &handle_) != 0 )
      exceptionHandler(newObjectV1C2<ETrRollback>(status, __PRETTY_FUNCTION__));
    lastRetainingTransaction_ = lrtRollback;
    startCount_--;
  }
  else if( startCount_ == 0 ){
    retainingHelper();
  }
  else{
    startCount_--;
    lastRetainingTransaction_ = lrtNone;
  }
  return *this;
}
//---------------------------------------------------------------------------
void Transaction::processingException(ksys::Exception * e)
{
  EClientServer * p = dynamic_cast< EClientServer *>(e);
  if( p != NULL ){
    if( p->isFatalError() ){
      while( active() ) rollback();
    }
  }
}
//---------------------------------------------------------------------------
void Transaction::staticExceptionHandler(ksys::Exception * e)
{
  processingException(e);
  for( intptr_t i = databases_.count() - 1; i >= 0; i-- )
    databases_[i]->processingException(e);
  e->throwSP();
}
//---------------------------------------------------------------------------
void Transaction::exceptionHandler(ksys::Exception * e)
{
  staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
// properties ///////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
TPB & Transaction::paramsByDatabase(Database & database)
{
  return *tpbs_.objectOfKey(utf8::ptr2Str(&database));
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
