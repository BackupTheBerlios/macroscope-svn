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
#include <adicpp/adicpp.h>
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
MYSQLDatabase::~MYSQLDatabase()
{
}
//---------------------------------------------------------------------------
MYSQLDatabase::MYSQLDatabase()
{
  static_cast<mycpp::Transaction *>(this)->attach(*this);
}
//---------------------------------------------------------------------------
void MYSQLDatabase::exceptionHandler(ksys::Exception * e)
{
  static_cast<mycpp::Database *>(this)->staticExceptionHandler(e);
}
//---------------------------------------------------------------------------
Statement * MYSQLDatabase::newStatement()
{
  return newObject<MYSQLStatement>();
}
//---------------------------------------------------------------------------
Statement * MYSQLDatabase::newAttachedStatement()
{
  ksys::AutoPtr<MYSQLStatement> p(newObject<MYSQLStatement>());
  p->attach(*this);
  return p.ptr(NULL);
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::create()
{
  static_cast<mycpp::Database *>(this)->create();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::drop()
{
  static_cast<mycpp::Database *>(this)->drop();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::attach()
{
  static_cast<mycpp::Database *>(this)->attach();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::detach()
{
  static_cast<mycpp::Database *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool MYSQLDatabase::attached()
{
  return static_cast<mycpp::Database *>(this)->attached();
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::start()
{
  static_cast<mycpp::Transaction *>(this)->start();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::rollback()
{
  static_cast<mycpp::Transaction *>(this)->rollback();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::commit()
{
  static_cast<mycpp::Transaction *>(this)->commit();
  return this;
}
//---------------------------------------------------------------------------
bool MYSQLDatabase::active()
{
  return static_cast<mycpp::Transaction *>(this)->active();
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::clearParams()
{
  static_cast<mycpp::Database *>(this)->params().clear();
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::addParam(const utf8::String & name, const ksys::Mutant & value)
{
  static_cast<mycpp::Database *>(this)->params().add(name, value);
  return this;
}
//---------------------------------------------------------------------------
MYSQLDatabase * MYSQLDatabase::name(const utf8::String & name)
{
  static_cast<mycpp::Database *>(this)->name(name);
  return this;
}
//---------------------------------------------------------------------------
utf8::String MYSQLDatabase::name()
{
  return static_cast<mycpp::Database *>(this)->name();
}
//---------------------------------------------------------------------------
bool MYSQLDatabase::separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port)
{
  return static_cast<mycpp::Database *>(this)->separateDBName(name, hostName, dbName, port);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
MYSQLStatement::~MYSQLStatement()
{
}
//---------------------------------------------------------------------------
MYSQLStatement::MYSQLStatement()
{
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::attach(Database & database)
{
  MYSQLDatabase * p = dynamic_cast<MYSQLDatabase *>(&database);
  if( p == NULL )
    newObjectV1C2<ksys::Exception>(EINVAL, __PRETTY_FUNCTION__)->throwSP();
  static_cast<mycpp::DSQLStatement *>(this)->attach(*p);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::detach()
{
  static_cast<mycpp::DSQLStatement *>(this)->detach();
  return this;
}
//---------------------------------------------------------------------------
bool MYSQLStatement::attached()
{
  return static_cast<mycpp::DSQLStatement *>(this)->attached();
}
//---------------------------------------------------------------------------
bool MYSQLStatement::prepared()
{
  return static_cast<mycpp::DSQLStatement *>(this)->prepared();
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::prepare()
{
  static_cast<mycpp::DSQLStatement *>(this)->prepare();
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::unprepare()
{
  static_cast<mycpp::DSQLStatement *>(this)->unprepare();
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::execute()
{
  static_cast<mycpp::DSQLStatement *>(this)->execute();
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::execute(const utf8::String & sqlTextImm)
{
  static_cast<mycpp::DSQLStatement *>(this)->execute(sqlTextImm);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::text(const utf8::String & sqlText)
{
  static_cast<mycpp::DSQLStatement *>(this)->sqlText(sqlText);
  return this;
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::text()
{
  return static_cast<mycpp::DSQLStatement *>(this)->sqlText();
}
//---------------------------------------------------------------------------
uintptr_t MYSQLStatement::paramCount()
{
  return static_cast<mycpp::DSQLStatement *>(this)->params().count();
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::paramName(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->params().paramName(i);
}
//---------------------------------------------------------------------------
ksys::Mutant MYSQLStatement::paramAsMutant(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->paramAsMutant(i);
}
//---------------------------------------------------------------------------
ksys::Mutant MYSQLStatement::paramAsMutant(const utf8::String & name)
{
  return static_cast<mycpp::DSQLStatement *>(this)->paramAsMutant(name);
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::paramAsString(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->paramAsString(i);
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::paramAsString(const utf8::String & name)
{
  return static_cast<mycpp::DSQLStatement *>(this)->paramAsString(name);
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::paramAsMutant(uintptr_t i, const ksys::Mutant & value)
{
  static_cast<mycpp::DSQLStatement *>(this)->paramAsMutant(i, value);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::paramAsMutant(const utf8::String & name, const ksys::Mutant & value)
{
  static_cast<mycpp::DSQLStatement *>(this)->paramAsMutant(name, value);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::paramAsString(uintptr_t i, const utf8::String & value)
{
  static_cast<mycpp::DSQLStatement *>(this)->paramAsString(i, value);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::paramAsString(const utf8::String & name, const utf8::String & value)
{
  static_cast<mycpp::DSQLStatement *>(this)->paramAsString(name, value);
  return this;
}
//---------------------------------------------------------------------------
bool MYSQLStatement::fetch()
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().fetch();
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::fetchAll()
{
  static_cast<mycpp::DSQLStatement *>(this)->values().fetchAll();
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::selectRow(uintptr_t i)
{
  static_cast<mycpp::DSQLStatement *>(this)->values().selectRow(i);
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::selectFirstRow()
{
  static_cast<mycpp::DSQLStatement *>(this)->values().selectFirst();
  return this;
}
//---------------------------------------------------------------------------
MYSQLStatement * MYSQLStatement::selectLastRow()
{
  static_cast<mycpp::DSQLStatement *>(this)->values().selectLast();
  return this;
}
//---------------------------------------------------------------------------
uintptr_t MYSQLStatement::rowCount()
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().rowCount();
}
//---------------------------------------------------------------------------
intptr_t MYSQLStatement::rowIndex()
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().rowIndex();
}
//---------------------------------------------------------------------------
ksys::Mutant MYSQLStatement::valueAsMutant(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->valueAsMutant(i);
}
//---------------------------------------------------------------------------
ksys::Mutant MYSQLStatement::valueAsMutant(const utf8::String & name)
{
  return static_cast<mycpp::DSQLStatement *>(this)->valueAsMutant(name);
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::valueAsString(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->valueAsString(i);
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::valueAsString(const utf8::String & name)
{
  return static_cast<mycpp::DSQLStatement *>(this)->valueAsString(name);
}
//---------------------------------------------------------------------------
bool MYSQLStatement::valueIsNull(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().isNull(i);
}
//---------------------------------------------------------------------------
bool MYSQLStatement::valueIsNull(const utf8::String & name)
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().isNull(name);
}
//---------------------------------------------------------------------------
FieldType MYSQLStatement::fieldType(uintptr_t i)
{
  i = static_cast<mycpp::DSQLStatement *>(this)->values().checkValueIndex(i);
  i = static_cast<mycpp::DSQLStatement *>(this)->values().field(i).type;
  switch( i ){
    case MYSQL_TYPE_DECIMAL     :
      return ftDouble;
    case MYSQL_TYPE_TINY        :
      return ftChar;
    case MYSQL_TYPE_SHORT       :
      return ftShort;
    case MYSQL_TYPE_LONG        :
      return ftInt;
    case MYSQL_TYPE_FLOAT       :
      return ftFloat;
    case MYSQL_TYPE_DOUBLE      :
      return ftDouble;
    case MYSQL_TYPE_NULL        :
      break;
    case MYSQL_TYPE_TIMESTAMP   :
      return ftTime;
    case MYSQL_TYPE_LONGLONG    :
      return ftLong;
    case MYSQL_TYPE_INT24       :
      return ftInt;
    case MYSQL_TYPE_DATE        :
    case MYSQL_TYPE_TIME        :
    case MYSQL_TYPE_DATETIME    :
    case MYSQL_TYPE_YEAR        :
    case MYSQL_TYPE_NEWDATE     :
      return ftTime;
    case MYSQL_TYPE_ENUM        :
    case MYSQL_TYPE_SET         :
      break;
    case MYSQL_TYPE_TINY_BLOB   :
    case MYSQL_TYPE_MEDIUM_BLOB :
    case MYSQL_TYPE_LONG_BLOB   :
    case MYSQL_TYPE_BLOB        :
      return ftBlob;
    case MYSQL_TYPE_VAR_STRING  :
    case MYSQL_TYPE_STRING      :
      return ftString;
    case MYSQL_TYPE_GEOMETRY :
      break;
    default :
      ;
  }
  return ftUnknown;
}
//---------------------------------------------------------------------------
uintptr_t MYSQLStatement::fieldCount()
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().count();
}
//---------------------------------------------------------------------------
FieldType MYSQLStatement::fieldType(const utf8::String & name)
{
  return fieldType(static_cast<mycpp::DSQLStatement *>(this)->values().indexOfName(name));
}
//---------------------------------------------------------------------------
utf8::String MYSQLStatement::fieldName(uintptr_t i)
{
  return static_cast<mycpp::DSQLStatement *>(this)->values().nameOfIndex(i);
}
//---------------------------------------------------------------------------
intptr_t MYSQLStatement::fieldIndex(const utf8::String & name,bool noThrow)
{
  intptr_t i = static_cast<mycpp::DSQLStatement *>(this)->values().indexOfName(name);
  if( i < 0 && !noThrow )
    newObjectV1C2<ksys::Exception>(EINVAL,"invalid field name: " + name + ", " + __PRETTY_FUNCTION__)->throwSP();
  return i;
}
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
