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
#include <adicpp/adicpp.h>
//---------------------------------------------------------------------------
namespace adicpp {

//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
FirebirdDatabase::~FirebirdDatabase()
{
}
//---------------------------------------------------------------------------
FirebirdDatabase::FirebirdDatabase()
{
  static_cast< fbcpp::Transaction *>(this)->attach(*this);
}
//---------------------------------------------------------------------------
void FirebirdDatabase::exceptionHandler(ksys::Exception * e)
{
  static_cast< fbcpp::Database *>(this)->staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
Statement * FirebirdDatabase::newStatement()
{
  return newObject<FirebirdStatement>();
}
//---------------------------------------------------------------------------
Statement * FirebirdDatabase::newAttachedStatement()
{
  ksys::AutoPtr<FirebirdStatement> p(newObject<FirebirdStatement>());
  p->attach(*this);
  return p.ptr(NULL);
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::create()
{
  static_cast< fbcpp::Database *>(this)->create();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::drop()
{
  static_cast< fbcpp::Database *>(this)->drop();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::attach()
{
  static_cast< fbcpp::Database *>(this)->attach();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::detach()
{
  static_cast< fbcpp::Database *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool FirebirdDatabase::attached()
{
  return static_cast< fbcpp::Database *>(this)->attached();
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::start()
{
  static_cast< fbcpp::Transaction *>(this)->start();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::rollback()
{
  static_cast< fbcpp::Transaction *>(this)->rollback();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::commit()
{
  static_cast< fbcpp::Transaction *>(this)->commit();
  return this;
}
//---------------------------------------------------------------------------
bool FirebirdDatabase::active()
{
  return static_cast< fbcpp::Transaction *>(this)->active();
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::clearParams()
{
  static_cast< fbcpp::Database *>(this)->params().clear();
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::addParam(const utf8::String & name, const ksys::Mutant & value)
{
  static_cast< fbcpp::Database *>(this)->params().add(name, value);
  return this;
}
//---------------------------------------------------------------------------
FirebirdDatabase * FirebirdDatabase::name(const utf8::String & name)
{
  static_cast< fbcpp::Database *>(this)->name(name);
  return this;
}
//---------------------------------------------------------------------------
bool FirebirdDatabase::separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port)
{
  return static_cast< fbcpp::Database *>(this)->separateDBName(name, hostName, dbName, port);
}
//---------------------------------------------------------------------------
utf8::String FirebirdDatabase::name()
{
  return static_cast< fbcpp::Database *>(this)->name();
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
FirebirdStatement::~FirebirdStatement()
{
}
//---------------------------------------------------------------------------
FirebirdStatement::FirebirdStatement()
{
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::attach(Database & database)
{
  FirebirdDatabase * p = dynamic_cast<FirebirdDatabase *>(&database);
  if( p == NULL )
    newObject<ksys::Exception>(EINVAL,__PRETTY_FUNCTION__)->throwSP();
  static_cast<fbcpp::DSQLStatement *>(this)->attach(*p,*p);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::detach()
{
  static_cast<fbcpp::DSQLStatement *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool FirebirdStatement::attached()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->attached();
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::prepare()
{
  static_cast< fbcpp::DSQLStatement *>(this)->prepare();
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::unprepare()
{
  static_cast< fbcpp::DSQLStatement *>(this)->unprepare();
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::execute()
{
  static_cast< fbcpp::DSQLStatement *>(this)->execute();
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::execute(const utf8::String & sqlTextImm)
{
  static_cast< fbcpp::DSQLStatement *>(this)->execute(sqlTextImm);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::text(const utf8::String & sqlText)
{
  static_cast< fbcpp::DSQLStatement *>(this)->sqlText(sqlText);
  return this;
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::text()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->sqlText();
}
//---------------------------------------------------------------------------
uintptr_t FirebirdStatement::paramCount()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->params().count();
}
//---------------------------------------------------------------------------
ksys::Mutant FirebirdStatement::paramAsMutant(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->paramAsMutant(i);
}
//---------------------------------------------------------------------------
ksys::Mutant FirebirdStatement::paramAsMutant(const utf8::String & name)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->paramAsMutant(name);
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::paramAsString(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->paramAsString(i);
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::paramAsString(const utf8::String & name)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->paramAsString(name);
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::paramAsMutant(uintptr_t i, const ksys::Mutant & value)
{
  static_cast< fbcpp::DSQLStatement *>(this)->paramAsMutant(i, value);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::paramAsMutant(const utf8::String & name, const ksys::Mutant & value)
{
  static_cast< fbcpp::DSQLStatement *>(this)->paramAsMutant(name, value);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::paramAsString(uintptr_t i, const utf8::String & value)
{
  static_cast< fbcpp::DSQLStatement *>(this)->paramAsString(i, value);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::paramAsString(const utf8::String & name, const utf8::String & value)
{
  static_cast< fbcpp::DSQLStatement *>(this)->paramAsString(name, value);
  return this;
}
//---------------------------------------------------------------------------
bool FirebirdStatement::fetch()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().fetch();
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::fetchAll()
{
  static_cast< fbcpp::DSQLStatement *>(this)->values().fetchAll();
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::selectRow(uintptr_t i)
{
  static_cast< fbcpp::DSQLStatement *>(this)->values().selectRow(i);
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::selectFirstRow()
{
  static_cast< fbcpp::DSQLStatement *>(this)->values().selectFirst();
  return this;
}
//---------------------------------------------------------------------------
FirebirdStatement * FirebirdStatement::selectLastRow()
{
  static_cast< fbcpp::DSQLStatement *>(this)->values().selectLast();
  return this;
}
//---------------------------------------------------------------------------
uintptr_t FirebirdStatement::rowCount()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().rowCount();
}
//---------------------------------------------------------------------------
intptr_t FirebirdStatement::rowIndex()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().rowIndex();
}
//---------------------------------------------------------------------------
uintptr_t FirebirdStatement::fieldCount()
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().count();
}
//---------------------------------------------------------------------------
ksys::Mutant FirebirdStatement::valueAsMutant(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->valueAsMutant(i);
}
//---------------------------------------------------------------------------
ksys::Mutant FirebirdStatement::valueAsMutant(const utf8::String & name)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->valueAsMutant(name);
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::valueAsString(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->valueAsString(i);
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::valueAsString(const utf8::String & name)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->valueAsString(name);
}
//---------------------------------------------------------------------------
bool FirebirdStatement::valueIsNull(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().isNull(i);
}
//---------------------------------------------------------------------------
bool FirebirdStatement::valueIsNull(const utf8::String & name)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().isNull(name);
}
//---------------------------------------------------------------------------
FieldType FirebirdStatement::fieldType(uintptr_t i)
{
  i = static_cast< fbcpp::DSQLStatement *>(this)->values().checkValueIndex(i);
  switch( static_cast< fbcpp::DSQLStatement *>(this)->
         values().sqlda().sqlda()->sqlvar[i].sqltype & ~1 ){
    case SQL_VARYING     :
    case SQL_TEXT        :
      return ftString;
    case SQL_SHORT       :
      return ftShort;
    case SQL_LONG        :
      return ftInt;
    case SQL_FLOAT       :
      return ftFloat;
    case SQL_DOUBLE      :
    case SQL_D_FLOAT     :
      return ftDouble;
    case SQL_TYPE_TIME   :
    case SQL_TYPE_DATE   :
    case SQL_TIMESTAMP   :
      return ftTime;
    case SQL_QUAD        :
    case SQL_INT64       :
      return ftLong;
    case SQL_BLOB        :
      return ftBlob;
    case SQL_ARRAY       :
      break;
  }
  return ftUnknown;
}
//---------------------------------------------------------------------------
FieldType FirebirdStatement::fieldType(const utf8::String & name)
{
  return fieldType(static_cast< fbcpp::DSQLStatement *>(this)->values().indexOfName(name));
}
//---------------------------------------------------------------------------
utf8::String FirebirdStatement::fieldName(uintptr_t i)
{
  return static_cast< fbcpp::DSQLStatement *>(this)->values().nameOfIndex(i);
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
