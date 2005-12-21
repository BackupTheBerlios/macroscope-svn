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
DPB::DPB()
{
}
//---------------------------------------------------------------------------
DPB::~DPB()
{
}
//---------------------------------------------------------------------------
DPB & DPB::clear()
{
  user_.resize(0);
  password_.resize(0);
  protocol_.resize(0);
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::add(const utf8::String & name,const ksys::Mutant & value)
{
  if( name.strcasecmp(utf8::string("user_name")) == 0 ) user_ = value;
  else
  if( name.strcasecmp(utf8::string("password")) == 0 ) password_ = value;
  else
  if( name.strcasecmp(utf8::string("protocol")) == 0 ) protocol_ = value;
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
Database::~Database()
{
  if( transaction_ != NULL ) transaction_->detach();
  detach();
}
//---------------------------------------------------------------------------
Database::Database() : handle_(NULL), transaction_(NULL)
{
  dsqlStatements_.ownsObjects(false);
}
//---------------------------------------------------------------------------
Database & Database::allocHandle(MYSQL * & handle)
{
  if( handle == NULL ) handle = api.mysql_init(NULL);
  if( handle == NULL )
    exceptionHandler(new EClientServer(-1,utf8::string(__PRETTY_FUNCTION__)));
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::freeHandle(MYSQL * & handle)
{
  if( handle != NULL ){
    api.mysql_close(handle);
    handle = NULL;
  }
  return *this;
}
//---------------------------------------------------------------------------
bool Database::separateDBName(
  const utf8::String & name,
  utf8::String & hostName,
  utf8::String & dbName,
  uintptr_t & port)
{
  utf8::String::Iterator i1(name);
  while( !i1.eof() && i1.getChar() != ':' ) i1.next();
  utf8::String::Iterator i2(i1 + 1);
  while( !i2.eof() && i2.getChar() != ':' ) i2.next();
  port = 0;
  if( i1.eof() && i2.eof() ){ // unix socket or windows named pipe
    dbName = name;
  }
  else {
    intmax_t prt;
    if( utf8::tryStr2Int(utf8::string(i1 + 1,i2),prt) && i2.eof() ){ // dbName and port
      dbName = utf8::string(utf8::String::Iterator(name),i1);
    }
    else {
      utf8::tryStr2Int(utf8::string(i2 + 1,utf8::String::Iterator(name).last()),prt);
      hostName = utf8::string(utf8::String::Iterator(name),i1);
      dbName = utf8::string(i1 + 1,i2);
    }
    port = (uintptr_t) prt;
  }
  return true;
}
//---------------------------------------------------------------------------
Database & Database::create(const utf8::String & name)
{
  utf8::String hostName, dbName;
  uintptr_t  port;
  separateDBName(name.strlen() > 0 ? name : name_,hostName,dbName,port);
  api.open();
  MYSQL * handle = NULL;
  try {
    allocHandle(handle);
    api.mysql_real_connect(
      handle,
      hostName.strlen() > 0 ? hostName.c_str() : NULL,
      dpb_.user().c_str(),
      dpb_.password().c_str(),
      NULL,
      (unsigned int) port,
      hostName.strlen() > 0 ? hostName.c_str() : NULL,
      CLIENT_COMPRESS
    );
    if( api.mysql_errno(handle) != 0 )
      exceptionHandler(
        new EDBAttach(
          api.mysql_errno(handle),
          utf8::string(api.mysql_error(handle))
        )
      );
    dbName = utf8::string("CREATE DATABASE ") + dbName + " DEFAULT CHARACTER SET UTF8";
    if( api.mysql_query(handle,dbName.c_str()) != 0 )
      if( api.mysql_errno(handle) != ER_DB_CREATE_EXISTS )
        exceptionHandler(
          new EDBCreate(
            api.mysql_errno(handle),
            utf8::string(api.mysql_error(handle))
          )
        );
  }
  catch( ksys::ExceptionSP & ){
    freeHandle(handle);
    api.close();
    throw;
  }
  freeHandle(handle);
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::drop()
{
  if( !attached() ) exceptionHandler(new EDBNotAttached(-1,utf8::string(__PRETTY_FUNCTION__)));
  if( transaction_ != NULL )
    while( transaction_->active() ) transaction_->rollback();
  for( intptr_t  i = dsqlStatements_.count() - 1; i >= 0; i-- )
    dsqlStatements_.objectOfIndex(i)->free();
  if( api.mysql_query(handle_,(utf8::string("DROP DATABASE ") + handle_->db).c_str()) != 0 )
    exceptionHandler(
      new EDBDrop(
      api.mysql_errno(handle_),
      utf8::string(api.mysql_error(handle_)))
    );
  freeHandle(handle_);
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::attach(const utf8::String & name)
{
  utf8::String hostName, dbName;
  uintptr_t  port;
  separateDBName(name.strlen() > 0 ? name : name_,hostName,dbName,port);
  if( !attached() ){
    api.open();
    try {
      allocHandle(handle_);
      unsigned int protocol = MYSQL_PROTOCOL_DEFAULT;
      if( dpb_.protocol().strcasecmp(utf8::string("DEFAULT")) == 0 ) protocol = MYSQL_PROTOCOL_DEFAULT;
      else
      if( dpb_.protocol().strcasecmp(utf8::string("TCP")) == 0 ) protocol = MYSQL_PROTOCOL_TCP;
      else
      if( dpb_.protocol().strcasecmp(utf8::string("PIPE")) == 0 ) protocol = MYSQL_PROTOCOL_PIPE;
      else
      if( dpb_.protocol().strcasecmp(utf8::string("MEMORY")) == 0 ) protocol = MYSQL_PROTOCOL_MEMORY;
      if( api.mysql_options(handle_,MYSQL_OPT_PROTOCOL,&protocol) != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );
      unsigned int timeout = 180;
      if( api.mysql_options(handle_,MYSQL_OPT_READ_TIMEOUT,&timeout) != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );
      if( api.mysql_options(handle_,MYSQL_OPT_WRITE_TIMEOUT,&timeout) != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );
      if( api.mysql_options(handle_,MYSQL_SET_CHARSET_NAME,"utf8") != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );

      api.mysql_real_connect(
        handle_,
        hostName.strlen() > 0 ? hostName.c_str() : NULL,
        dpb_.user().c_str(),
        dpb_.password().c_str(),
        dbName.c_str(),
        (unsigned int) port,
        hostName.strlen() > 0 ? hostName.c_str() : NULL,
        0//CLIENT_COMPRESS | CLIENT_MULTI_STATEMENTS
      );
      if( api.mysql_errno(handle_) != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );
      if( api.mysql_autocommit(handle_,false) != 0 )
        exceptionHandler(
          new EDBAttach(
            api.mysql_errno(handle_),
            utf8::string(api.mysql_error(handle_))
          )
        );
    }
    catch( ksys::ExceptionSP & ){
      freeHandle(handle_);
      api.close();
      throw;
    }
    if( name.strlen() > 0 ) name_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::detach()
{
  if( attached() ){
//    throw EDBDetach(this,-1,utf8::string(__PRETTY_FUNCTION__));
    if( transaction_ != NULL )
      while( transaction_->active() ) transaction_->rollback();
    for( intptr_t  i = dsqlStatements_.count() - 1; i >= 0; i-- )
      dsqlStatements_.objectOfIndex(i)->free();
    freeHandle(handle_);
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
    }
  }
}
//---------------------------------------------------------------------------
void Database::staticExceptionHandler(ksys::Exception * e)
{
  transaction_->processingException(e);
  processingException(e);
  throw ksys::ExceptionSP(e);
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
Database & Database::name(const utf8::String & name)
{
  if( name_.strcasecmp(name) != 0 ){
    if( attached() ) detach();
    name_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace fbcpp
//---------------------------------------------------------------------------
