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
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
DPB::~DPB()
{
}
//---------------------------------------------------------------------------
DPB::DPB()
{
}
//---------------------------------------------------------------------------
DPB & DPB::clear()
{
  user_.resize(0);
  password_.resize(0);
  return *this;
}
//---------------------------------------------------------------------------
DPB & DPB::add(const utf8::String & name, const ksys::Mutant & value)
{
  if( name.strcasecmp("user_name") == 0 ) user_ = value;
  else if( name.strcasecmp("password") == 0 ) password_ = value;
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
Database::Database() : handle_(NULL),  transaction_(NULL)
{
}
//---------------------------------------------------------------------------
bool Database::separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port)
{
  utf8::String::Iterator i1(name);
  while( !i1.eof() && i1.getChar() != ':' ) i1.next();
  utf8::String::Iterator  i2  (i1 + 1);
  while( !i2.eof() && i2.getChar() != ':' ) i2.next();
  port = 0;
  if( i1.eof() && i2.eof() ){
    // unix socket or windows named pipe
    dbName = name;
  }
  else {
    intmax_t  prt;
    if( utf8::tryStr2Int(utf8::String(i1 + 1, i2), prt) && i2.eof() ){
      // dbName and port
      dbName = utf8::String(utf8::String::Iterator(name), i1);
    }
    else {
      utf8::tryStr2Int(utf8::String(i2 + 1, utf8::String::Iterator(name).last()), prt);
      hostName = utf8::String(utf8::String::Iterator(name), i1);
      dbName = utf8::String(i1 + 1, i2);
    }
    port = (uintptr_t) prt;
  }
  return true;
}
//---------------------------------------------------------------------------
Database & Database::create(const utf8::String & name)
{
  utf8::String  hostName, dbName;
  uintptr_t     port;
  separateDBName(name.strlen() > 0 ? name : name_, hostName, dbName, port);
  api.open();
  try {
    SQLRETURN r = api.SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&envHandle_);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ){
      printf("Error AllocHandle\n");
      exit(0);
    }
    r = api.SQLSetEnvAttr(&envHandle_,SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3,0);
    if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ){
      printf("Error SetEnv\n");
      api.SQLFreeHandle(SQL_HANDLE_ENV,envHandle_);
      exit(0);
    }
//    exceptionHandler(newObjectV1C2<EDBCreate>(api.mysql_errno(handle), api.mysql_error(handle)));
  }
  catch( ksys::ExceptionSP & ){
    api.close();
    throw;
  }
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::drop()
{
/*  if( !attached() )
    exceptionHandler(newObjectV1C2<EDBNotAttached>(EINVAL, __PRETTY_FUNCTION__));
  if( transaction_ != NULL )
    while( transaction_->active() ) transaction_->rollback();
  for( intptr_t  i = dsqlStatements_.count() - 1; i >= 0; i-- )
    dsqlStatements_.objectOfIndex(i)->free();
  if( api.mysql_query(handle_, (utf8::String("DROP DATABASE ") + handle_->db).c_str()) != 0 )
    exceptionHandler(newObjectV1C2<EDBDrop>(api.mysql_errno(handle_), api.mysql_error(handle_)));
  freeHandle(handle_);*/
  api.close();
  return *this;
}
//---------------------------------------------------------------------------
Database & Database::attach(const utf8::String & name)
{
  utf8::String  hostName, dbName;
  uintptr_t     port;
  separateDBName(name.strlen() > 0 ? name : name_, hostName, dbName, port);
  if( !attached() ){
    api.open();
    try {
      SQLRETURN r = api.SQLAllocHandle(SQL_HANDLE_ENV,SQL_NULL_HANDLE,&envHandle_);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ){
        printf("Error AllocHandle\n");
	      exit(0);
      }
      r = api.SQLSetEnvAttr(&envHandle_,SQL_ATTR_ODBC_VERSION,(SQLPOINTER) SQL_OV_ODBC3,0);
      if( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ){
        printf("Error SetEnv\n");
	      api.SQLFreeHandle(SQL_HANDLE_ENV,envHandle_);
        exit(0);
      }
//      exceptionHandler(newObjectV1C2<EClientServer>(api.mysql_errno(handle_), api.mysql_error(handle_)));
    }
    catch( ksys::ExceptionSP & ){
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
/*  if( attached() ){
    if( transaction_ != NULL )
      while( transaction_->active() ) transaction_->rollback();
    for( intptr_t i = dsqlStatements_.count() - 1; i >= 0; i-- )
      dsqlStatements_.objectOfIndex(i)->free();
    freeHandle(handle_);
    api.close();
  }*/
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
Database & Database::name(const utf8::String & name)
{
  if( name_.strcasecmp(name) != 0 ){
    if( attached() ) detach();
    name_ = name;
  }
  return *this;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
