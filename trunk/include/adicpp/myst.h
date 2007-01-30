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
#ifndef _myst_H_
#define _myst_H_
//---------------------------------------------------------------------------
namespace mycpp {
//---------------------------------------------------------------------------
utf8::String  time2Str(int64_t t);
int64_t       str2Time(const char * s);
inline int64_t str2Time(const utf8::String & s)
{
  return str2Time(s.c_str());
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class MYSQL_BIND_Holder {
  public:
    ~MYSQL_BIND_Holder();
    MYSQL_BIND_Holder();

    MYSQL_BIND_Holder &   resize(long n);
    // properties
    MYSQL_BIND * const &  bind();
    const uintptr_t &     count();
  private:
    MYSQL_BIND *  bind_;
    uintptr_t     count_;
};
//---------------------------------------------------------------------------
inline MYSQL_BIND_Holder::MYSQL_BIND_Holder() : bind_(NULL), count_(0)
{
}
//---------------------------------------------------------------------------
inline MYSQL_BIND_Holder::~MYSQL_BIND_Holder()
{
  ksys::xfree(bind_);
}
//---------------------------------------------------------------------------
inline MYSQL_BIND_Holder & MYSQL_BIND_Holder::resize(long count)
{
  ksys::xrealloc(bind_, sizeof(MYSQL_BIND) * count);
  count_ = count;
  return *this;
}
//---------------------------------------------------------------------------
inline MYSQL_BIND * const & MYSQL_BIND_Holder::bind()
{
  return bind_;
}
//---------------------------------------------------------------------------
inline const uintptr_t & MYSQL_BIND_Holder::count()
{
  return count_;
}
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
        MYSQL_TIME  timestamp_;
    };
    unsigned long     length_;
    bool              changed_;
    my_bool           isNull_;
    ksys::MutantType  type_;

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
inline DSQLParam::DSQLParam(DSQLStatement & statement)
  : statement_(&statement),
    changed_(false),
    type_(ksys::mtNull)
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
    MYSQL_BIND_Holder                                                   bind_;
    ksys::HashedObjectList< utf8::String,DSQLParam>                     params_;
    ksys::Array< ksys::HashedObjectListItem< utf8::String,DSQLParam> *> indexToParam_;

    DSQLParams &                                          bind();
    DSQLParams &                                          resetChanged();
    DSQLParams &                                          removeUnchanged();
    ksys::HashedObjectListItem< utf8::String,DSQLParam> * add(const utf8::String & paramName);
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
  params_.caseSensitive(false);
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLParams::count()
{
  return params_.count();
}
//---------------------------------------------------------------------------
inline utf8::String DSQLParams::paramName(uintptr_t i)
{
  return checkParamIndex(i).params_.keyOfIndex(i);
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class DSQLRow {
  friend class DSQLValues;
  public:
    ~DSQLRow();
    DSQLRow();
  protected:
    ksys::AutoPtr<uint8_t> raw_;
    ksys::Array<int32_t>   index_;
    ksys::Array<my_bool>   isNulls_;
    uint32_t               rowSize_;
  private:
};
//---------------------------------------------------------------------------
inline DSQLRow::DSQLRow() : rowSize_(0)
{
}
//---------------------------------------------------------------------------
inline DSQLRow::~DSQLRow()
{
}
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

    const MYSQL_FIELD & field(uintptr_t i);
  protected:
  private:
    DSQLStatement *                                 statement_;
    MYSQL_BIND_Holder                               bind_;
    MYSQL_RES *                                     res_;
    MYSQL_FIELD *                                   fields_;
    ksys::Array< unsigned long>                     lengths_;
    ksys::Vector< DSQLRow>                          rows_;
    intptr_t                                        row_;
    ksys::HashedObjectList<utf8::String,int32_t>    valuesIndex_;

    DSQLValues &  clear();
    DSQLRow *     bind();
    DSQLValues &  fillRow(DSQLRow * row);
    struct fillRowVars {
        union {
            char *              pc;
            int8_t *            pb;
            int16_t *           ps;
            uint16_t *          pus;
            int32_t *           pl;
            int64_t *           pll;
            float *             pf;
            double *            pd;
            const MYSQL_TIME *  stamp;
        };
        intptr_t      i, j, k;
        MYSQL_BIND *  rbind;
        DSQLRow *     row;
    };

    DSQLValues &        fillRowHelper(struct fillRowVars & v, uintptr_t ds);
    intptr_t            getValueIndex(const utf8::String & name);
    DSQLValues &        freeRes();
};
//---------------------------------------------------------------------------
inline DSQLValues::DSQLValues(DSQLStatement & statement)
  : statement_(&statement), res_(NULL), row_(-1)
{
  valuesIndex_.caseSensitive(false);
  valuesIndex_.ownsObjects(false);
}
//---------------------------------------------------------------------------
inline DSQLValues::~DSQLValues()
{
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::selectRow(uintptr_t row)
{
  row_ = checkRowIndex(row);
  return *this;
}
//---------------------------------------------------------------------------
inline intptr_t valueIndexComparator(const utf8::String & s1, const utf8::String & s2)
{
  return s1.strcasecmp(s2);
}
//---------------------------------------------------------------------------
inline intptr_t DSQLValues::getValueIndex(const utf8::String & name)
{
  return valuesIndex_.indexOfKey(name);
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::clear()
{
  rows_.clear();
  row_ = -1;
  return *this;
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLValues::rowCount()
{
  return rows_.count();
}
//---------------------------------------------------------------------------
inline intptr_t DSQLValues::rowIndex()
{
  return row_;
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::selectFirst()
{
  return selectRow(0);
}
//---------------------------------------------------------------------------
inline DSQLValues & DSQLValues::selectLast()
{
  return selectRow(rowCount() - 1);
}
//---------------------------------------------------------------------------
inline uintptr_t DSQLValues::count()
{
  return bind_.count();
}
//---------------------------------------------------------------------------
inline intptr_t DSQLValues::indexOfName(const utf8::String & name)
{
  return getValueIndex(name);
}
//---------------------------------------------------------------------------
inline const utf8::String DSQLValues::nameOfIndex(uintptr_t i)
{
  return valuesIndex_.keyOfIndex(checkValueIndex(i));
}
//---------------------------------------------------------------------------
inline bool DSQLValues::isNull(uintptr_t i)
{
  return rows_[checkRowIndex(row_)].index_[checkValueIndex(i)] < 0;
}
//---------------------------------------------------------------------------
inline bool DSQLValues::isNull(const utf8::String & name)
{
  return rows_[checkRowIndex(row_)].index_[checkValueIndex(getValueIndex(name))] < 0;
}
//---------------------------------------------------------------------------
inline ksys::Mutant DSQLValues::asMutant(const utf8::String & name)
{
  return asMutant(getValueIndex(name));
}
//---------------------------------------------------------------------------
inline utf8::String DSQLValues::asString(const utf8::String & name)
{
  return asString(getValueIndex(name));
}
//---------------------------------------------------------------------------
inline const MYSQL_FIELD & DSQLValues::field(uintptr_t i)
{
  return fields_[checkValueIndex(i)];
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
    MYSQL_STMT *    handle();
    bool            attached();
    bool            allocated();
    bool &          storeResults();
    utf8::String    sqlText();
    DSQLStatement & sqlText(const utf8::String & sqlText);
    DSQLParams &    params();
    DSQLValues &    values();
  protected:
  private:
    MYSQL_STMT *  handle_;
    Database *    database_;
    utf8::String  sqlText_;
    bool          sqlTextChanged_;
    bool          prepared_;
    bool          executed_;
    bool          storeResults_;
    DSQLParams    params_;
    DSQLValues    values_;

    bool            isSQLTextDDL() const;
    DSQLStatement & allocate();
    DSQLStatement & free();
    utf8::String    compileSQLParameters();
};
//---------------------------------------------------------------------------
inline int64_t DSQLStatement::insertId()
{
  return api.mysql_insert_id(database_->handle_);
}
//---------------------------------------------------------------------------
inline MYSQL_STMT * DSQLStatement::handle()
{
  return handle_;
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
inline ksys::Mutant DSQLStatement::paramAsMutant(uintptr_t i)
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
}
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
} // namespace mycpp
//---------------------------------------------------------------------------
#endif /* _myst_H_ */
//---------------------------------------------------------------------------
