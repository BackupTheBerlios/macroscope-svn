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
#ifndef adistH
#define adistH
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
enum FieldType {
  ftChar,
  ftShort,
  ftInt,    // 32 bit integer
  ftLong,   // 64 bit integer
  ftFloat,  // 32 bit float
  ftDouble, // 64 bit float
  ftTime,
  ftString,
  ftBlob,
  ftUnknown
};
//---------------------------------------------------------------------------
class Statement : virtual public ksys::Object {
  public:
    virtual ~Statement() {}
    Statement() {}

    void beforeDestruction() { detach(); }
    
    virtual Statement *                 attach(Database & database) = 0;
    virtual Statement *                 detach() = 0;
    virtual bool                        attached() = 0;

    virtual Statement *                 prepare() = 0;
    virtual Statement *                 unprepare() = 0;
    virtual Statement *                 execute() = 0;
    virtual Statement *                 execute(const utf8::String & sqlTextImm) = 0;
    virtual Statement *                 text(const utf8::String & sqlText) = 0;
    virtual utf8::String                text() = 0;

    // query parameters methods
    virtual uintptr_t                   paramCount() = 0;
    virtual utf8::String                paramName(uintptr_t i) = 0;
    virtual ksys::Mutant                paramAsMutant(uintptr_t i) = 0;
    virtual ksys::Mutant                paramAsMutant(const utf8::String & name) = 0;
    virtual utf8::String                paramAsString(uintptr_t i) = 0;
    virtual utf8::String                paramAsString(const utf8::String & name) = 0;

    virtual Statement *                 paramAsMutant(uintptr_t i, const ksys::Mutant & value) = 0;
    virtual Statement *                 paramAsMutant(const utf8::String & name, const ksys::Mutant & value) = 0;
    virtual Statement *                 paramAsString(uintptr_t i, const utf8::String & value) = 0;
    virtual Statement *                 paramAsString(const utf8::String & name, const utf8::String & value) = 0;

    // query result set access methods
    virtual bool                        fetch() = 0;
    virtual Statement *                 fetchAll() = 0;
    virtual Statement *                 selectRow(uintptr_t i) = 0;
    virtual Statement *                 selectFirstRow() = 0;
    virtual Statement *                 selectLastRow() = 0;
    virtual uintptr_t                   rowCount() = 0;
    virtual intptr_t                    rowIndex() = 0;
    virtual uintptr_t                   fieldCount() = 0;
    virtual ksys::Mutant                valueAsMutant(uintptr_t i) = 0;
    virtual ksys::Mutant                valueAsMutant(const utf8::String & name) = 0;
    virtual utf8::String                valueAsString(uintptr_t i) = 0;
    virtual utf8::String                valueAsString(const utf8::String & name) = 0;

    virtual bool                        valueIsNull(uintptr_t i) = 0;
    virtual bool                        valueIsNull(const utf8::String & name) = 0;

    virtual FieldType                   fieldType(uintptr_t i) = 0;
    virtual FieldType                   fieldType(const utf8::String & name) = 0;
    virtual utf8::String                fieldName(uintptr_t i) = 0;

    template <class Table> Statement & unload(Table & table);
  protected:
  private:
};
//---------------------------------------------------------------------------
template <class Table>
#ifndef __BCPLUSPLUS__
inline
#endif
Statement & Statement::unload(Table & table)
{
  intptr_t  i, j, k, row;
  table.clear();
  for( j = fieldCount(), i = 0; i < j; i++ )
    table.addColumn(fieldName(i));
  for( k = rowCount(), row = 0; row < k; row++ ){
    selectRow(row);
    table.addRow();
    for( i = 0; i < j; i++ )
      table(row, i) = valueAsMutant(i);
  }
  return *this;
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FirebirdStatement : public Statement, public fbcpp::DSQLStatement {
  public:
    virtual ~FirebirdStatement();
    FirebirdStatement();

    void beforeDestruction() { detach(); }

    FirebirdStatement * attach(Database & database);
    FirebirdStatement * detach();
    bool                attached();

    FirebirdStatement * prepare();
    FirebirdStatement * unprepare();
    FirebirdStatement * execute();
    FirebirdStatement * execute(const utf8::String & sqlTextImm);
    FirebirdStatement * text(const utf8::String & sqlText);
    utf8::String        text();

    // query parameters methods
    uintptr_t           paramCount();
    utf8::String        paramName(uintptr_t i);
    ksys::Mutant        paramAsMutant(uintptr_t i);
    ksys::Mutant        paramAsMutant(const utf8::String & name);
    utf8::String        paramAsString(uintptr_t i);
    utf8::String        paramAsString(const utf8::String & name);

    FirebirdStatement * paramAsMutant(uintptr_t i, const ksys::Mutant & value);
    FirebirdStatement * paramAsMutant(const utf8::String & name, const ksys::Mutant & value);
    FirebirdStatement * paramAsString(uintptr_t i, const utf8::String & value);
    FirebirdStatement * paramAsString(const utf8::String & name, const utf8::String & value);

    // query result set access methods
    bool                fetch();
    FirebirdStatement * fetchAll();
    FirebirdStatement * selectRow(uintptr_t i);
    FirebirdStatement * selectFirstRow();
    FirebirdStatement * selectLastRow();
    uintptr_t           rowCount();
    intptr_t            rowIndex();
    uintptr_t           fieldCount();
    ksys::Mutant        valueAsMutant(uintptr_t i);
    ksys::Mutant        valueAsMutant(const utf8::String & name);
    utf8::String        valueAsString(uintptr_t i);
    utf8::String        valueAsString(const utf8::String & name);

    bool                valueIsNull(uintptr_t i);
    bool                valueIsNull(const utf8::String & name);

    FieldType           fieldType(uintptr_t i);
    FieldType           fieldType(const utf8::String & name);
    utf8::String        fieldName(uintptr_t i);
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class MYSQLStatement : public Statement, public mycpp::DSQLStatement {
  public:
    ~MYSQLStatement();
    MYSQLStatement();

    void beforeDestruction() { detach(); }

    MYSQLStatement *  attach(Database & database);
    MYSQLStatement *  detach();
    bool              attached();

    MYSQLStatement *  prepare();
    MYSQLStatement *  unprepare();
    MYSQLStatement *  execute();
    MYSQLStatement *  execute(const utf8::String & sqlTextImm);
    MYSQLStatement *  text(const utf8::String & sqlText);
    utf8::String      text();

    // query parameters methods
    uintptr_t         paramCount();
    utf8::String      paramName(uintptr_t i);
    ksys::Mutant      paramAsMutant(uintptr_t i);
    ksys::Mutant      paramAsMutant(const utf8::String & name);
    utf8::String      paramAsString(uintptr_t i);
    utf8::String      paramAsString(const utf8::String & name);

    MYSQLStatement *  paramAsMutant(uintptr_t i, const ksys::Mutant & value);
    MYSQLStatement *  paramAsMutant(const utf8::String & name, const ksys::Mutant & value);
    MYSQLStatement *  paramAsString(uintptr_t i, const utf8::String & value);
    MYSQLStatement *  paramAsString(const utf8::String & name, const utf8::String & value);

    // query result set access methods
    bool              fetch();
    MYSQLStatement *  fetchAll();
    MYSQLStatement *  selectRow(uintptr_t i);
    MYSQLStatement *  selectFirstRow();
    MYSQLStatement *  selectLastRow();
    uintptr_t         rowCount();
    intptr_t          rowIndex();
    uintptr_t         fieldCount();
    ksys::Mutant      valueAsMutant(uintptr_t i);
    ksys::Mutant      valueAsMutant(const utf8::String & name);
    utf8::String      valueAsString(uintptr_t i);
    utf8::String      valueAsString(const utf8::String & name);

    bool              valueIsNull(uintptr_t i);
    bool              valueIsNull(const utf8::String & name);

    FieldType         fieldType(uintptr_t i);
    FieldType         fieldType(const utf8::String & name);
    utf8::String      fieldName(uintptr_t i);
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
#endif // adidbH
