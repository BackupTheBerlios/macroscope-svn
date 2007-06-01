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
#ifndef _odbcst_H_
#define _odbcst_H_
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Base : virtual ksys::Object {
  friend class EClientServer;
  public:
    virtual ~Base() {}
    Base() {}
  protected:
    virtual void exceptionHandler(ksys::Exception * e) = 0;
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParam {
  friend class DSQLParams;
  friend class DSQLStatement;
  public:
    ~DSQLParam();
    DSQLParam() {}
    DSQLParam(DSQLStatement & statement);
  protected:
    DSQLStatement       * statement_;
    utf8::String        string_;
    ksys::MemoryStream  stream_;
    union {
      int64_t     int_;
      double      float_;
    };
    ksys::MutantType type_;

    ksys::Mutant  getMutant();
    DSQLParam &   setMutant(const ksys::Mutant & value);
    utf8::String  getString();
    DSQLParam &   setString(const utf8::String & value);
  private:
    DSQLParam(const DSQLParam &){}
    void operator = (const DSQLParam &){}
};
//---------------------------------------------------------------------------
inline DSQLParam::~DSQLParam()
{
}
//---------------------------------------------------------------------------
inline DSQLParam::DSQLParam(DSQLStatement & statement) :
  statement_(&statement), type_(ksys::mtNull)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLParams {
  friend class DSQLStatement;
  public:
    ~DSQLParams();
    DSQLParams() {}
    DSQLParams(DSQLStatement & statement);

    uintptr_t count();
    utf8::String paramName(uintptr_t i);
    intptr_t paramIndex(const utf8::String & name);
    // access methods
    bool                                                  isNull(uintptr_t i);
    DSQLParams &                                          setNull(uintptr_t i);
    bool                                                  isNull(const utf8::String & paramName);
    DSQLParams &                                          setNull(const utf8::String & paramName);

    ksys::Mutant                                          asMutant(uintptr_t i);
    DSQLParams &                                          asMutant(uintptr_t i, const ksys::Mutant & value);
    utf8::String                                          asString(uintptr_t i);
    DSQLParams &                                          asString(uintptr_t i, const utf8::String & value);

    ksys::Mutant                                          asMutant(const utf8::String & paramName);
    DSQLParams &                                          asMutant(const utf8::String & paramName, const ksys::Mutant & value);
    utf8::String                                          asString(const utf8::String & paramName);
    DSQLParams &                                          asString(const utf8::String & paramName, const utf8::String & value);
  protected:
  private:
    DSQLStatement * statement_;
    DSQLParams &                                          checkParamIndex(uintptr_t i);
    DSQLParam *                                           checkParamName(const utf8::String & paramName);
};
//---------------------------------------------------------------------------
inline DSQLParams::~DSQLParams()
{
}
//---------------------------------------------------------------------------
inline DSQLParams::DSQLParams(DSQLStatement & statement) : statement_(&statement)
{
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLParams::count()
{
  return -1;
}
//---------------------------------------------------------------------------
/*inline utf8::String DSQLParams::paramName(uintptr_t i)
{
  return checkParamIndex(i).params_.keyOfIndex(i);
}
//---------------------------------------------------------------------------
inline intptr_t DSQLParams::paramIndex(const utf8::String & name)
{
  return params_.indexOfKey(name);
}*/
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLValues {
  friend class DSQLStatement;
  friend class Transaction;
  public:
    ~DSQLValues();
    DSQLValues() {}
    DSQLValues(DSQLStatement & statement);

    uintptr_t           rowCount();
    intptr_t            rowIndex();
    bool                fetch();
    DSQLValues &        fetchAll();
    DSQLValues &        selectRow(uintptr_t row);
    DSQLValues &        selectFirst();
    DSQLValues &        selectLast();

    uintptr_t           count();
    uintptr_t           checkRowIndex(uintptr_t row);
    uintptr_t           checkValueIndex(uintptr_t i);
    intptr_t            indexOfName(const utf8::String & name);
    const utf8::String  nameOfIndex(uintptr_t i);

    // access methods
    bool                isNull(uintptr_t i);
    bool                isNull(const utf8::String & name);

    ksys::Mutant        asMutant(uintptr_t i);
    ksys::Mutant        asMutant(const utf8::String & name);
    utf8::String        asString(uintptr_t i);
    utf8::String        asString(const utf8::String & name);

  protected:
  private:
    DSQLStatement *                                 statement_;
};
//---------------------------------------------------------------------------
inline DSQLValues::~DSQLValues()
{
}
//---------------------------------------------------------------------------
inline DSQLValues::DSQLValues(DSQLStatement & statement)
  : statement_(&statement)
{
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLStatement : virtual public ksys::Object {
  friend class Database;
  friend class Transaction;
  friend class DSQLParams;
  friend class DSQLValues;
  public:
    virtual ~DSQLStatement();
    DSQLStatement();

    void beforeDestruction() { detach(); }
    
    DSQLStatement & attach(Database & database);
    DSQLStatement & detach();

    DSQLStatement & prepare();
    DSQLStatement & unprepare();
    DSQLStatement & execute();
    DSQLStatement & execute(const utf8::String & sqlTextImm);

    ksys::Mutant    paramAsMutant(uintptr_t i);
    ksys::Mutant    paramAsMutant(const utf8::String & name);
    utf8::String    paramAsString(uintptr_t i);
    utf8::String    paramAsString(const utf8::String & name);

    DSQLStatement & paramAsMutant(uintptr_t i, const ksys::Mutant & value);
    DSQLStatement & paramAsMutant(const utf8::String & name, const ksys::Mutant & value);
    DSQLStatement & paramAsString(uintptr_t i, const utf8::String & value);
    DSQLStatement & paramAsString(const utf8::String & name, const utf8::String & value);

    ksys::Mutant    valueAsMutant(uintptr_t i);
    ksys::Mutant    valueAsMutant(const utf8::String & name);
    utf8::String    valueAsString(uintptr_t i);
    utf8::String    valueAsString(const utf8::String & name);

    int64_t         insertId();

    // properties
    bool            attached();
    bool            allocated();
    bool &          storeResults();
    const bool & prepared() const;
    utf8::String    sqlText();
    DSQLStatement & sqlText(const utf8::String & sqlText);
    DSQLParams &    params();
    DSQLValues &    values();
    Database * database() const;
  protected:
  private:
    static ksys::EmbeddedListNode<DSQLStatement> & listNode(const DSQLStatement & object){
      return object.listNode_;
    }
    static DSQLStatement & listObject(const ksys::EmbeddedListNode<DSQLStatement> & node,DSQLStatement * p = NULL){
      return node.object(p->listNode_);
    }
    mutable ksys::EmbeddedListNode<DSQLStatement> listNode_;

    Database *    database_;
    SQLHANDLE * handle_;
    utf8::String  sqlText_;
    bool          sqlTextChanged_;
    bool          prepared_;
    bool          executed_;
    bool          storeResults_;
    DSQLParams    params_;
    DSQLValues    values_;

    DSQLStatement & allocate();
    DSQLStatement & free();
    utf8::String    compileSQLParameters();
};
//---------------------------------------------------------------------------
inline Database * DSQLStatement::database() const
{
  return database_;
}
//---------------------------------------------------------------------------
inline bool DSQLStatement::attached()
{
  return database_ != NULL;
}
//---------------------------------------------------------------------------
inline bool DSQLStatement::allocated()
{
  return handle_ != NULL;
}
//---------------------------------------------------------------------------
inline bool & DSQLStatement::storeResults()
{
  return storeResults_;
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::sqlText()
{
  return sqlText_;
}
//---------------------------------------------------------------------------
inline DSQLParams & DSQLStatement::params()
{
  return params_;
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLStatement::values()
{
  return values_;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::execute(const utf8::String & sqlTextImm)
{
  return sqlText(sqlTextImm).execute();
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::unprepare()
{
  prepared_ = false;
  return *this;
}
//---------------------------------------------------------------------------
inline const bool & DSQLStatement::prepared() const
{
  return prepared_;
}
//---------------------------------------------------------------------------
/*inline ksys::Mutant DSQLStatement::paramAsMutant(uintptr_t i)
{
  return params_.asMutant(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::paramAsMutant(const utf8::String & name)
{
  return params_.asMutant(name);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::paramAsString(uintptr_t i)
{
  return params_.asString(i);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::paramAsString(const utf8::String & name)
{
  return params_.asString(name);
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsMutant(uintptr_t i, const ksys::Mutant & value)
{
  params_.asMutant(i, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsMutant(const utf8::String & name, const ksys::Mutant & value)
{
  params_.asMutant(name, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsString(uintptr_t i, const utf8::String & value)
{
  params_.asString(i, value);
  return *this;
}
//---------------------------------------------------------------------------
inline DSQLStatement & DSQLStatement::paramAsString(const utf8::String & name, const utf8::String & value)
{
  params_.asString(name, value);
  return *this;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::valueAsMutant(uintptr_t i)
{
  return values_.asMutant(i);
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLStatement::valueAsMutant(const utf8::String & name)
{
  return values_.asMutant(name);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::valueAsString(uintptr_t i)
{
  return values_.asString(i);
}
//---------------------------------------------------------------------------
inline utf8::String DSQLStatement::valueAsString(const utf8::String & name)
{
  return values_.asString(name);
} */
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif /* _odbcst_H_ */
//---------------------------------------------------------------------------
