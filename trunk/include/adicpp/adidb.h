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
#ifndef adidbH
#define adidbH
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option push -w-inl
#endif
//---------------------------------------------------------------------------
namespace adicpp {
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Statement;
//---------------------------------------------------------------------------
class Database : virtual public ksys::Object {
  public:
    virtual ~Database() {}
    Database() {}

    
    //void beforeDestruction() { detach(); }

    static Database *     newDatabase(const ksys::ConfigSection * config);

    virtual Statement *   newStatement() = 0;
    virtual Statement *   newAttachedStatement() = 0;

    virtual Database *    create() = 0;
    virtual Database *    drop() = 0;
    virtual Database *    attach() = 0;
    virtual Database *    detach() = 0;
    virtual bool          attached() = 0;

    virtual Database *    isolation(const utf8::String & isolation) = 0;
    virtual utf8::String  isolation() = 0;
    virtual Database *    start() = 0;
    virtual Database *    rollback(bool noThrow = false) = 0;
    virtual Database *    commit(bool noThrow = false) = 0;
    virtual bool          active() = 0;

    virtual Database *    clearParams() = 0;
    virtual Database *    addParam(const utf8::String & name, const ksys::Mutant & value) = 0;

    virtual Database *    name(const utf8::String & name) = 0;
    virtual utf8::String  name() = 0;
    virtual bool          separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port) = 0;

    // metadata manipulation ?
    //    virtual Database & createTable(const utf8::String & tableName,const utf8::String & fields) = 0;
    //    virtual Database & alterTable(const utf8::String & tableName,const utf8::String & newName) = 0;
    //    virtual Database & dropTable(const utf8::String & tableName) = 0;
    //    virtual Database & createFields(const utf8::String & tableName,const utf8::String & fields) = 0;
    //    virtual Database & alterFields(const utf8::String & tableName,const utf8::String & fields) = 0;
    //    virtual Database & dropFields(const utf8::String & tableName,const utf8::String & fields) = 0;
  protected:
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class AutoDatabaseDetach {
  public:
    ~AutoDatabaseDetach(){ database_->detach(); }
    AutoDatabaseDetach(Database & database) : database_(&database) { database_->attach(); }
  protected:
    Database * database_;
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class FirebirdDatabase : public Database, public fbcpp::Database, public fbcpp::Transaction {
  public:
    virtual ~FirebirdDatabase();
    FirebirdDatabase();

    void beforeDestruction() { fbcpp::Transaction::detach(); fbcpp::Database::detach(); }

    Statement *         newStatement();
    Statement *         newAttachedStatement();

    FirebirdDatabase *  create();
    FirebirdDatabase *  drop();
    FirebirdDatabase *  attach();
    FirebirdDatabase *  detach();
    bool                attached();

    FirebirdDatabase *  isolation(const utf8::String & isolation);
    utf8::String        isolation();
    FirebirdDatabase *  start();
    FirebirdDatabase *  rollback(bool noThrow);
    FirebirdDatabase *  commit(bool noThrow);
    bool                active();

    FirebirdDatabase *  clearParams();
    FirebirdDatabase *  addParam(const utf8::String & name, const ksys::Mutant & value);

    FirebirdDatabase *  name(const utf8::String & name);
    utf8::String        name();
    bool                separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port);
  protected:
    void                exceptionHandler(ksys::Exception * e);
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class MYSQLDatabase : public Database, public mycpp::Database, public mycpp::Transaction {
  public:
    virtual ~MYSQLDatabase();
    MYSQLDatabase();

    void beforeDestruction() { mycpp::Transaction::detach(); mycpp::Database::detach(); }

    Statement *     newStatement();
    Statement *     newAttachedStatement();

    MYSQLDatabase * create();
    MYSQLDatabase * drop();
    MYSQLDatabase * attach();
    MYSQLDatabase * detach();
    bool            attached();

    MYSQLDatabase * isolation(const utf8::String & isolation);
    utf8::String    isolation();
    MYSQLDatabase * start();
    MYSQLDatabase * rollback(bool noThrow);
    MYSQLDatabase * commit(bool noThrow);
    bool            active();

    MYSQLDatabase * clearParams();
    MYSQLDatabase * addParam(const utf8::String & name, const ksys::Mutant & value);

    MYSQLDatabase * name(const utf8::String & name);
    utf8::String    name();
    bool            separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port);
  protected:
    void            exceptionHandler(ksys::Exception * e);
  private:
};
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class ODBCDatabase : public Database, public odbcpp::Database, public odbcpp::Transaction {
  public:
    virtual ~ODBCDatabase();
    ODBCDatabase();

    void beforeDestruction() { odbcpp::Transaction::detach(); odbcpp::Database::detach(); }

    Statement *     newStatement();
    Statement *     newAttachedStatement();

    ODBCDatabase *  create();
    ODBCDatabase *  drop();
    ODBCDatabase *  attach();
    ODBCDatabase *  detach();
    bool            attached();

    ODBCDatabase *  isolation(const utf8::String & isolation);
    utf8::String    isolation();
    ODBCDatabase *  start();
    ODBCDatabase *  rollback(bool noThrow);
    ODBCDatabase *  commit(bool noThrow);
    bool            active();

    ODBCDatabase *  clearParams();
    ODBCDatabase *  addParam(const utf8::String & name, const ksys::Mutant & value);

    ODBCDatabase *  name(const utf8::String & name);
    utf8::String    name();
    bool            separateDBName(const utf8::String & name, utf8::String & hostName, utf8::String & dbName, uintptr_t & port);
  protected:
    void            exceptionHandler(ksys::Exception * e);
  private:
};
//---------------------------------------------------------------------------
} // namespace adicpp
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option pop
#endif
//---------------------------------------------------------------------------
#endif // adidbH
//---------------------------------------------------------------------------
