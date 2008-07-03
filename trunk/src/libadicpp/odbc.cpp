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
#include <adicpp/adicpp.h>
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ODBCDatabase::~ODBCDatabase()
{
}
//---------------------------------------------------------------------------
ODBCDatabase::ODBCDatabase()
{
  static_cast<odbcpp::Transaction *>(this)->attach(*this);
}
//---------------------------------------------------------------------------
void ODBCDatabase::exceptionHandler(ksys::Exception * e)
{
  static_cast<odbcpp::Database *>(this)->staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
Statement * ODBCDatabase::newStatement()
{
  return newObject<ODBCStatement>();
}
//---------------------------------------------------------------------------
Statement * ODBCDatabase::newAttachedStatement()
{
  ksys::AutoPtr<ODBCStatement> p(newObject<ODBCStatement>());
  p->attach(*this);
  return p.ptr(NULL);
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::create()
{
  static_cast<odbcpp::Database *>(this)->create();
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::drop()
{
  static_cast<odbcpp::Database *>(this)->drop();
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::attach()
{
  static_cast<odbcpp::Database *>(this)->attach();
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::detach()
{
  static_cast<odbcpp::Database *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool ODBCDatabase::attached()
{
  return static_cast<odbcpp::Database *>(this)->attached();
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::isolation(const utf8::String & isolation)
{
  static_cast<odbcpp::Transaction *>(this)->isolation(isolation);
  return this;
}
//---------------------------------------------------------------------------
utf8::String ODBCDatabase::isolation()
{
  return static_cast<odbcpp::Transaction *>(this)->isolation();
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::start()
{
  static_cast<odbcpp::Transaction *>(this)->start();
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::rollback(bool noThrow)
{
  static_cast<odbcpp::Transaction *>(this)->rollback(noThrow);
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::commit(bool noThrow)
{
  static_cast<odbcpp::Transaction *>(this)->commit(noThrow);
  return this;
}
//---------------------------------------------------------------------------
bool ODBCDatabase::active()
{
  return static_cast<odbcpp::Transaction *>(this)->active();
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::clearParams()
{
  newObjectV1C2<ksys::Exception>(ENOSYS,__PRETTY_FUNCTION__);
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::addParam(const utf8::String & name, const ksys::Mutant & value)
{
  newObjectV1C2<ksys::Exception>(ENOSYS,__PRETTY_FUNCTION__);
  return this;
}
//---------------------------------------------------------------------------
ODBCDatabase * ODBCDatabase::name(const utf8::String & name)
{
  static_cast<odbcpp::Database *>(this)->connection(name);
  return this;
}
//---------------------------------------------------------------------------
utf8::String ODBCDatabase::name()
{
  return static_cast<odbcpp::Database *>(this)->connection();
}
//---------------------------------------------------------------------------
bool ODBCDatabase::separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port)
{
  newObjectV1C2<ksys::Exception>(ENOSYS,__PRETTY_FUNCTION__);
  return false;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
ODBCStatement::~ODBCStatement()
{
}
//---------------------------------------------------------------------------
ODBCStatement::ODBCStatement()
{
}
//---------------------------------------------------------------------------
Database * ODBCStatement::database()
{
  return dynamic_cast<adicpp::Database *>(static_cast<odbcpp::DSQLStatement *>(this)->database());
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::attach(Database & database)
{
  ODBCDatabase * p = dynamic_cast<ODBCDatabase *>(&database);
  if( p == NULL )
    newObjectV1C2<ksys::Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  static_cast<odbcpp::DSQLStatement *>(this)->attach(*p);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::detach()
{
  static_cast<odbcpp::DSQLStatement *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool ODBCStatement::attached()
{
  return static_cast<odbcpp::DSQLStatement *>(this)->attached();
}
//---------------------------------------------------------------------------
bool ODBCStatement::prepared()
{
  return static_cast<odbcpp::DSQLStatement *>(this)->prepared();
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::prepare()
{
  static_cast<odbcpp::DSQLStatement *>(this)->prepare();
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::unprepare()
{
  static_cast<odbcpp::DSQLStatement *>(this)->unprepare();
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::execute()
{
  static_cast<odbcpp::DSQLStatement *>(this)->execute();
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::execute(const utf8::String & sqlTextImm)
{
  static_cast<odbcpp::DSQLStatement *>(this)->execute(sqlTextImm);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::text(const utf8::String & sqlText)
{
  static_cast<odbcpp::DSQLStatement *>(this)->sqlText(sqlText);
  return this;
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::text()
{
  return static_cast<odbcpp::DSQLStatement *>(this)->sqlText();
}
//---------------------------------------------------------------------------
uintptr_t ODBCStatement::paramCount()
{
  return static_cast<odbcpp::DSQLStatement *>(this)->params().count();
}
//---------------------------------------------------------------------------
intptr_t ODBCStatement::paramIndex(const utf8::String & name,bool noThrow)
{
  /*intptr_t i = static_cast<odbcpp::DSQLStatement *>(this)->params().paramIndex(name);
  if( i < 0 && !noThrow )
    newObjectV1C2<ksys::Exception>(EINVAL,"invalid param name: " + name + ", " + __PRETTY_FUNCTION__)->throwSP();
  return i;*/
  return -1;
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::paramName(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->params().paramName(i);
  return utf8::String();
}
//---------------------------------------------------------------------------
ksys::Mutant ODBCStatement::paramAsMutant(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->paramAsMutant(i);
  return utf8::String();
}
//---------------------------------------------------------------------------
ksys::Mutant ODBCStatement::paramAsMutant(const utf8::String & name)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->paramAsMutant(name);
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::paramAsString(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->paramAsString(i);
  return utf8::String();
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::paramAsString(const utf8::String & name)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->paramAsString(name);
  return utf8::String();
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::paramAsMutant(uintptr_t i, const ksys::Mutant & value)
{
//  static_cast<odbcpp::DSQLStatement *>(this)->paramAsMutant(i, value);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::paramAsMutant(const utf8::String & name, const ksys::Mutant & value)
{
//  static_cast<odbcpp::DSQLStatement *>(this)->paramAsMutant(name, value);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::paramAsString(uintptr_t i, const utf8::String & value)
{
//  static_cast<odbcpp::DSQLStatement *>(this)->paramAsString(i, value);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::paramAsString(const utf8::String & name, const utf8::String & value)
{
//  static_cast<odbcpp::DSQLStatement *>(this)->paramAsString(name, value);
  return this;
}
//---------------------------------------------------------------------------
bool ODBCStatement::fetch()
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().fetch();
  return false;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::fetchAll()
{
//  static_cast<odbcpp::DSQLStatement *>(this)->values().fetchAll();
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::selectRow(uintptr_t i)
{
//  static_cast<odbcpp::DSQLStatement *>(this)->values().selectRow(i);
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::selectFirstRow()
{
//  static_cast<odbcpp::DSQLStatement *>(this)->values().selectFirst();
  return this;
}
//---------------------------------------------------------------------------
ODBCStatement * ODBCStatement::selectLastRow()
{
//  static_cast<odbcpp::DSQLStatement *>(this)->values().selectLast();
  return this;
}
//---------------------------------------------------------------------------
uintptr_t ODBCStatement::rowCount()
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().rowCount();
  return 0;
}
//---------------------------------------------------------------------------
intptr_t ODBCStatement::rowIndex()
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().rowIndex();
  return -1;
}
//---------------------------------------------------------------------------
ksys::Mutant ODBCStatement::valueAsMutant(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->valueAsMutant(i);
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
ksys::Mutant ODBCStatement::valueAsMutant(const utf8::String & name)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->valueAsMutant(name);
  return ksys::Mutant();
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::valueAsString(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->valueAsString(i);
  return utf8::String();
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::valueAsString(const utf8::String & name)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->valueAsString(name);
  return utf8::String();
}
//---------------------------------------------------------------------------
bool ODBCStatement::valueIsNull(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().isNull(i);
  return false;
}
//---------------------------------------------------------------------------
bool ODBCStatement::valueIsNull(const utf8::String & name)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().isNull(name);
  return false;
}
//---------------------------------------------------------------------------
FieldType ODBCStatement::fieldType(uintptr_t i)
{
/*  i = static_cast<odbcpp::DSQLStatement *>(this)->values().checkValueIndex(i);
  i = static_cast<odbcpp::DSQLStatement *>(this)->values().field(i).type;
  switch( i ){
    case ODBC_TYPE_DECIMAL     :
      return ftDouble;
    case ODBC_TYPE_TINY        :
      return ftChar;
    case ODBC_TYPE_SHORT       :
      return ftShort;
    case ODBC_TYPE_LONG        :
      return ftInt;
    case ODBC_TYPE_FLOAT       :
      return ftFloat;
    case ODBC_TYPE_DOUBLE      :
      return ftDouble;
    case ODBC_TYPE_NULL        :
      break;
    case ODBC_TYPE_TIMESTAMP   :
      return ftTime;
    case ODBC_TYPE_LONGLONG    :
      return ftLong;
    case ODBC_TYPE_INT24       :
      return ftInt;
    case ODBC_TYPE_DATE        :
    case ODBC_TYPE_TIME        :
    case ODBC_TYPE_DATETIME    :
    case ODBC_TYPE_YEAR        :
    case ODBC_TYPE_NEWDATE     :
      return ftTime;
    case ODBC_TYPE_ENUM        :
    case ODBC_TYPE_SET         :
      break;
    case ODBC_TYPE_TINY_BLOB   :
    case ODBC_TYPE_MEDIUM_BLOB :
    case ODBC_TYPE_LONG_BLOB   :
    case ODBC_TYPE_BLOB        :
      return ftBlob;
    case ODBC_TYPE_VAR_STRING  :
    case ODBC_TYPE_STRING      :
      return ftString;
    case ODBC_TYPE_GEOMETRY :
      break;
    default :
      ;
  }*/
  return ftUnknown;
}
//---------------------------------------------------------------------------
uintptr_t ODBCStatement::fieldCount()
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().count();
  return 0;
}
//---------------------------------------------------------------------------
FieldType ODBCStatement::fieldType(const utf8::String & name)
{
//  return fieldType(static_cast<odbcpp::DSQLStatement *>(this)->values().indexOfName(name));
  return ftUnknown;
}
//---------------------------------------------------------------------------
utf8::String ODBCStatement::fieldName(uintptr_t i)
{
//  return static_cast<odbcpp::DSQLStatement *>(this)->values().nameOfIndex(i);
  return utf8::String();
}
//---------------------------------------------------------------------------
intptr_t ODBCStatement::fieldIndex(const utf8::String & name,bool noThrow)
{
  /*intptr_t i = static_cast<odbcpp::DSQLStatement *>(this)->values().indexOfName(name);
  if( i < 0 && !noThrow )
    newObjectV1C2<ksys::Exception>(EINVAL,"invalid field name: " + name + ", " + __PRETTY_FUNCTION__)->throwSP();
  return i;*/
  return -1;
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
