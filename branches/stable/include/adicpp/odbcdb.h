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
#ifndef odbcdbH
#define odbcdbH
//---------------------------------------------------------------------------
namespace odbcpp {
//---------------------------------------------------------------------------
class Database;
class DSQLStatement;
//---------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------
class Database : virtual public Base {
  friend class Transaction;
  friend class DSQLStatement;
  public:
    virtual ~Database();
    Database();
    
    void beforeDestruction() { detach(); }

    Database &            create(const utf8::String & name = utf8::String());
    Database &            drop();
    Database &            attach(const utf8::String & name = utf8::String());
    Database &            detach();

    // properties
    bool                  attached();
    Database &            connection(const utf8::String & name);
    const utf8::String &  connection();

    //  protected:
    void                  processingException(ksys::Exception * e);
    void                  staticExceptionHandler(ksys::Exception * e);
    void                  exceptionHandler(ksys::Exception * e);
  private:
    void * envHandle_; // Handle ODBC environment
    void * handle_;    // Handle connection
    utf8::String connection_;
    utf8::String database_;
    Transaction * transaction_;
    ksys::EmbeddedList<
      DSQLStatement,
      DSQLStatement::listNode,
      DSQLStatement::listObject      
    > statements_;
    Database & freeHandle();
    EClientServer * exception(int16_t handleType,void * handle,utf8::String * pSqlState = NULL) const;
};
//---------------------------------------------------------------------------
inline bool Database::attached()
{
  return handle_ != NULL;
}
//---------------------------------------------------------------------------
inline const utf8::String & Database::connection()
{
  return connection_;
}
//---------------------------------------------------------------------------
} // namespace odbcpp
//---------------------------------------------------------------------------
#endif // odbcdbH
//---------------------------------------------------------------------------
